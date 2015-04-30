#include "../include/konig.hpp"

int main(){
	RangeSampler sampler(10, 0, 100);
	for (auto val: sampler)
		std::cout << val << " ";
	std::cout << std::endl;

	IotaLabeler labeler;
	NoWeighter weighter;
    UndirectedGraph<int> g(100000, labeler, weighter);
    g.add_edges(100000);
    g.connect();
    std::cout << g << std::endl;
}
