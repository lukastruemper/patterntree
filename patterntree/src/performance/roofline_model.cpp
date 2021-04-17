#include "roofline_model.h"

#include <algorithm>
#include <functional>
#include <map>

#include "cluster/cluster.h"

double PatternTree::RooflineModel::runtime()
{
   return this->runtime_;
};

double PatternTree::RooflineModel::execution_costs(const std::vector<std::reference_wrapper<const PatternTree::PatternSplit>>& splits, const PatternTree::Team& team)
{
   double total_costs = 0.0;
   for (auto const& split : splits)
   {
      if ( split.get().width() == 0) { continue; } 

      double flops = split.get().flops();
      int width = split.get().width();
      
      double costs = flops;
      costs /= std::min(width, team.cores());
      //costs /= team.processor().arithmetic_units();
      costs /= team.processor().frequency() * FREQUENCY_TO_SECONDS;

      total_costs += costs;
   }

   return total_costs;
};

double PatternTree::RooflineModel::network_costs(const std::vector<std::reference_wrapper<const PatternTree::PatternSplit>>& splits, const PatternTree::Team& team)
{
   double initial_kbytes = 0;
   std::map<const PatternTree::Processor*, double> kbytes_transfer_table;
   
   std::unordered_set<std::shared_ptr<PatternTree::IView>> basis_views;
   for (auto const& split : splits)
   {
      for (auto const& view : split.get().consumes())
      {
         auto bvs = PatternTree::IView::as_basis(*view);
         basis_views.insert(bvs.begin(), bvs.end());
      }
   }

   for (auto const& basis_view : basis_views)
   {
      auto owners = this->state_.owned_by(*basis_view);
      double kbytes = basis_view->kbytes();
      auto range = owners.equal_range(basis_view);

      if (range.first == owners.end()) {
         initial_kbytes += kbytes;
      } else {
         std::vector<const PatternTree::Processor*> processors;
         for (auto it = range.first; it != range.second; it++)
         {
            processors.push_back(it->second);
         }

         const PatternTree::Processor* closest = &PatternTree::Cluster::closest(team.processor(), processors);  
         if (kbytes_transfer_table.find(closest) != kbytes_transfer_table.end()) {
            kbytes += kbytes_transfer_table[closest];
         }
         kbytes_transfer_table[closest] = kbytes;
      }
   }

   double total_costs = 0.0;
   if (initial_kbytes > 0.0) {
      double bandwidth;
      double latency;
      if (team.processor().device().type() == "CPU") {
         bandwidth = std::min(team.cores() * team.processor().device().memory_bandwidth(), team.processor().device().memory_max_bandwidth());
         latency = team.processor().device().memory_latency();
      } else {
         auto device = team.processor().device();
         auto node = device.node();
         auto cpu = node.devices().find("CPU1")->second;
         bandwidth = node.bandwidth(*cpu, device);
         latency = node.latency(*cpu, device);
      }

      total_costs += (latency / LATENCY_TO_SECONDS);
      total_costs += initial_kbytes / (bandwidth * BANDWIDTH_TO_SECONDS);
   }

   for (auto const& entry : kbytes_transfer_table)
   {
      auto dist = PatternTree::Cluster::distance(*entry.first, team.processor());
      switch (dist)
      {
      case PatternTree::Cluster::Distance::PROCESSOR:
      {
         double cache_bandwidth = team.cores() * team.processor().cache_bandwidth();
         double cache_latency = team.processor().cache_latency();

         double cached_kbytes = std::min(entry.second, team.processor().cache_size() * 1000.0);
         double main_kbytes = entry.second - cached_kbytes;

         double costs = cache_latency / LATENCY_TO_SECONDS;
         costs += cached_kbytes / (cache_bandwidth * BANDWIDTH_TO_SECONDS);
         if (main_kbytes > 0.0) {
            auto device = team.processor().device();
            double main_bandwidth = std::min(team.cores() * device.memory_bandwidth(), device.memory_max_bandwidth());
            double main_latency = device.memory_latency();

            double main_costs = main_kbytes / (main_bandwidth * BANDWIDTH_TO_SECONDS);
            main_costs += main_latency / LATENCY_TO_SECONDS;

            costs = std::max(costs, main_costs);
         }

         total_costs += costs;
         break;
      }
      case PatternTree::Cluster::Distance::DEVICE:
      {
         auto device = team.processor().device();
         double bandwidth = std::min(team.cores() * device.memory_bandwidth(), device.memory_max_bandwidth());
         double latency = device.memory_latency();

         double costs = latency / LATENCY_TO_SECONDS;
         costs += entry.second / (bandwidth * BANDWIDTH_TO_SECONDS);

         total_costs += costs;
         break;
      }
      case PatternTree::Cluster::Distance::NODE:
      {
         auto device = team.processor().device();
         auto node = device.node();
         double bandwidth = node.bandwidth(entry.first->device(), device);
         double latency = node.latency(entry.first->device(), device);

         double costs = latency / LATENCY_TO_SECONDS;
         costs += entry.second / (bandwidth * BANDWIDTH_TO_SECONDS);

         total_costs += costs;
         break;
      }
      default:
      {
         auto node = team.processor().device().node();
         auto cluster = node.cluster();
         double bandwidth = cluster.bandwidth(entry.first->device().node(), node);
         double latency = cluster.latency(entry.first->device().node(), node);

         double costs = latency / LATENCY_TO_SECONDS;
         costs += entry.second / (bandwidth * BANDWIDTH_TO_SECONDS);

         total_costs += costs;
         break;
      }
      }
   }

   return total_costs;
};

void PatternTree::RooflineModel::update(PatternTree::Step& step)
{
   double max_costs = 0.0;
   double max_exec_costs = 0.0;
   double max_net_costs = 0.0;

   for (auto const& team : step.teams()) {
      auto patterns = step.assigned(*team);

      double exec_costs = this->execution_costs(patterns, *team);
      double net_costs = this->network_costs(patterns, *team);
      double total_costs = (1.0 - std::min(net_costs / exec_costs, ROOFLINE_OVERLAP)) * exec_costs + net_costs;

      if (total_costs > max_costs) {
            max_costs = total_costs;
            max_exec_costs = exec_costs;
            max_net_costs = net_costs;
      }
   }

   this->runtime_ += max_costs;
   this->costs_.push_back(std::make_pair(max_exec_costs, max_net_costs));
   this->state_.update(step);
};
