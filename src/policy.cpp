//
// Created by victoryang00 on 1/12/23.
//

#include "policy.h"
#include <numeric>
Policy::Policy() {}
InterleavePolicy::InterleavePolicy() {}
// If the number is -1 for local, else it is the index of the remote server
int InterleavePolicy::compute_once(CXLController *controller) {
    auto per_size = controller->is_page ? 4096 : 64;
    if (controller->occupation.size() * per_size / 1024 / 1024 < controller->capacity) {
        return -1;
    } else {
        if (this->percentage.empty()) {
            // Here to compute the distributor statically using geometry average of write latency
            std::vector<double> to_store;
            for (auto &i : controller->cur_expanders) {
                to_store.push_back(1 / i->latency.write);
            }
            for (auto &i : to_store) {
                this->percentage.push_back(int(i / std::accumulate(to_store.begin(), to_store.end(), 0.0) * 10));
            }
            this->all_size = std::accumulate(this->percentage.begin(), this->percentage.end(), 0);
        }
        last_remote = (last_remote + 1) % all_size;
        int sum,index;
        for (index = 0, sum = 0; sum <= last_remote; index++) {
            sum += this->percentage[index];
            if (sum > last_remote) {
                break;
            }
        }
        return index;
    }
}
