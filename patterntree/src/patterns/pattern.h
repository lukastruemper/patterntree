#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "data/view.h"
#include "data/data.h"

namespace PatternTree
{
typedef std::vector<std::shared_ptr<IView>> Dataflow;

struct PatternIndexInfo {
	int index;
	size_t flops;
	std::unordered_map<IData*, std::shared_ptr<IView>> subviews;	
};

class IPattern {
std::string identifier_;
int width_;
Dataflow flow_in_;
Dataflow flow_out_;

protected:
	std::map<int, PatternIndexInfo> info_;

public:
	IPattern(std::string identifier, Dataflow, Dataflow, int width);

	std::string identifier() const;
	int width() const;
	Dataflow consumes() const;
	Dataflow produces() const;

	double flops() const;
	double flops(const int index, bool touch);
	std::shared_ptr<IView> subflow_in(const int index, IData& data);
	virtual std::shared_ptr<IView> subflow_out(const int index) = 0;

	virtual void touch(const int index) = 0;
};

}
