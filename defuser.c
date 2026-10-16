#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t phase4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t key) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + UINT32_C(0xa9)) ^ key;

    uint32_t child;
    uint32_t branch;
    if (target < middle) {
        child = phase4_walk(target, low, middle - 1u, key);
        branch = UINT32_C(9);
    } else {
        child = phase4_walk(target, middle + 1u, high, key);
        branch = UINT32_C(7);
    }
    return (5u * child) ^ (middle + key) ^ branch;
}

static uint32_t rotate_left_2(uint32_t value) {
    return (value << 2) | (value >> 30);
}

static uint32_t rotate_right_3(uint32_t value) {
    return (value >> 3) | (value << 29);
}

static uint32_t phase6_apply(uint32_t index, uint32_t value) {
    switch (index) {
        case 0:
        case 2:
            return rotate_left_2(value) ^ UINT32_C(0x2b5f);
        case 1:
        case 7:
            return rotate_right_3(value);
        case 3:
            return (8u * value + UINT32_C(0x6928)) ^ (5u * value);
        case 4:
            return (value + UINT32_C(0x62d1)) ^ (value >> 2);
        case 5:
            return (value ^ UINT32_C(0xd927)) + (value << 7);
        case 6:
            return ((value >> 7) ^ value) + UINT32_C(0x59c1);
        default:
            return 0;
    }
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);

    uint32_t step0 = 3u * (low - high) + UINT32_C(0x4b6f);
    uint32_t step1 = (middle + UINT32_C(0x9e38)) ^ high;
    uint32_t step2 = (step1 + 8u * low) ^ UINT32_C(0x80b8);

    values[0] = step2 ^ step0;
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = (uint32_t)((challenge >> 23) & UINT64_C(0x1f));
    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t part = (uint32_t)((challenge >> (9u + i)) & UINT64_C(0x3f));
        values[i] = (values[i - 1] + part) ^ (13u * i);
    }
    size_t count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t selector = (uint32_t)challenge & UINT32_C(7);
    uint32_t argument = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t result;

    switch (selector) {
        case 0:
            result = 3u * argument + UINT32_C(0xb15b);
            break;
        case 1:
            result = (argument ^ UINT32_C(0x4863)) + (argument >> 1);
            break;
        case 2:
            result = 5u * argument + UINT32_C(0x2d1a4);
            break;
        case 3:
            result = 3u * argument + UINT32_C(0x7e41);
            break;
        case 4:
            result = (4u * argument) ^ UINT32_C(0xc6fd);
            break;
        case 5:
            result = 5u * argument + UINT32_C(0xbfd1);
            break;
        case 6:
            result = (4u * argument) ^ UINT32_C(0xc88e);
            break;
        case 7:
            result = (argument ^ UINT32_C(0xbd18)) + (argument >> 1);
            break;
        default:
            return 1;
    }

    values[0] = selector;
    values[1] = result;
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)challenge & UINT32_C(0x7f);
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase4_walk(target, 0, UINT32_C(0x7f), key);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    static const uint32_t node_values[12] = {
        0xa284, 0x972f, 0x4c26, 0x893b, 0x1554, 0x0194,
        0xff42, 0x40a7, 0x95d6, 0x2c33, 0x6eee, 0x19f1,
    };
    static const uint32_t node_tags[12] = {
        0x0780, 0x0462, 0x017d, 0x0621, 0x0039, 0x0b2a,
        0x0963, 0x0886, 0x0a3a, 0x0533, 0x0215, 0x0354,
    };
    static const uint32_t node_next[12] = {
        6, 9, UINT32_MAX, 4, 5, 1, 7, 10, 11, 0, 8, 2,
    };
    static const uint32_t heads[8] = {3, 4, 5, 1, 9, 0, 6, 7};

    uint32_t node = heads[(uint32_t)challenge & UINT32_C(7)];
    uint32_t steps = (uint32_t)(challenge >> 3) & UINT32_C(3);
    uint32_t fold = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    fold = 3u * (fold ^ UINT32_C(0xc007)) + node_values[node];
    fold ^= node_tags[node];

    for (uint32_t i = 0; i < steps; ++i) {
        node = node_next[node];
        fold = 3u * fold + node_values[node];
        fold ^= node_tags[node];
    }

    values[0] = fold;
    values[1] = node_tags[node];
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t argument = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t first = phase6_apply(low & UINT32_C(7), argument) ^ UINT32_C(0x23e);
    values[0] = phase6_apply((low >> 3) & UINT32_C(7), first) + UINT32_C(0xab1b);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t choice = (uint32_t)context & UINT32_C(3);
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t mixed = 7u * (challenge_fold ^ context_fold) + UINT32_C(0x1eb2);
    mixed ^= UINT32_C(0x9561);

    uint32_t second = mixed ^ (choice * UINT32_C(0x1eb2))
                      ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (context_fold + choice + UINT32_C(0x9561)
                               + (challenge_fold >> 7));

    int size = snprintf(out, cap,
                        "................................OPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        choice, second, third);
    return size < 0 || (size_t)size >= cap;
}
