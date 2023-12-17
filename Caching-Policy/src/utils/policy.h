#ifndef _POLICY_HPP_INCLUDED_
#define _POLICY_HPP_INCLUDED_
enum class PolicyType {
    RANDOM,
    FIFO,
    LFU,
    LRU,
    LIRS,
    ARC,
    CLOCKPRO,
    TINYLFU
};

PolicyType policyTypes[] = {
    PolicyType::RANDOM,
    PolicyType::FIFO,
    PolicyType::LFU,
    PolicyType::LRU,
    PolicyType::LIRS,
    PolicyType::ARC,
    PolicyType::CLOCKPRO,
    PolicyType::TINYLFU
};
#endif /*_POLICY_INCLUDED_*/
