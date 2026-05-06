#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCS 15
#define MAX_RES  15

int p_to_r[MAX_PROCS][MAX_RES];
int r_holder[MAX_RES];

void init() {
    for (int i = 0; i < MAX_PROCS; i++)
        for (int j = 0; j < MAX_RES; j++)
            p_to_r[i][j] = 0;
    for (int i = 0; i < MAX_RES; i++)
        r_holder[i] = -1;
}

int cycle_start_process = -1;
int cycle_end_node = -1;
int parent_unified[MAX_PROCS + MAX_RES];

int dfs_unified(int start, int current, int state[]) {
    state[current] = 1;
    if (current < MAX_PROCS) {
        int p = current;
        for (int r = 0; r < MAX_RES; r++) {
            if (p_to_r[p][r]) {
                int res_node = MAX_PROCS + r;
                if (state[res_node] == 1) {
                    parent_unified[res_node] = current;
                    cycle_start_process = start;
                    cycle_end_node = res_node;
                    return 1;
                }
                if (state[res_node] == 0) {
                    parent_unified[res_node] = current;
                    if (dfs_unified(start, res_node, state))
                        return 1;
                }
            }
        }
    } else {
        int r = current - MAX_PROCS;
        int holder = r_holder[r];
        if (holder != -1) {
            if (state[holder] == 1) {
                parent_unified[holder] = current;
                cycle_start_process = start;
                cycle_end_node = holder;
                return 1;
            }
            if (state[holder] == 0) {
                parent_unified[holder] = current;
                if (dfs_unified(start, holder, state))
                    return 1;
            }
        }
    }
    state[current] = 2;
    return 0;
}

int detect_and_print_cycle(int start_proc) {
    int state[MAX_PROCS + MAX_RES];
    for (int i = 0; i < MAX_PROCS + MAX_RES; i++) {
        state[i] = 0;
        parent_unified[i] = -1;
    }
    if (dfs_unified(start_proc, start_proc, state)) {
        int cycle[100];
        int idx = 0;
        int cur = cycle_end_node;
        do {
            cycle[idx++] = cur;
            cur = parent_unified[cur];
        } while (cur != cycle_end_node);
        for (int i = 0; i < idx/2; i++) {
            int tmp = cycle[i];
            cycle[i] = cycle[idx-1-i];
            cycle[idx-1-i] = tmp;
        }
        for (int i = 0; i < idx; i++) {
            int u = cycle[i];
            int v = cycle[(i+1)%idx];
            if (u < MAX_PROCS) {
                int res = v - MAX_PROCS;
                printf("PROCESS %d -> RESOURCE %d\n", u, res);
            } else {
                int res = u - MAX_PROCS;
                printf("RESOURCE %d -> PROCESS %d\n", res, v);
            }
        }
        return 1;
    }
    return 0;
}

void handle_acquire(int p, int r) {
    if (r_holder[r] == -1) {
        r_holder[r] = p;
        if (detect_and_print_cycle(p)) {
            exit(-1);
        }
    } else {
        p_to_r[p][r] = 1;
        if (detect_and_print_cycle(p)) {
            exit(-1);
        }
    }
}

void handle_release(int p, int r) {
    if (r_holder[r] != p) return;
    r_holder[r] = -1;
    int new_holder = -1;
    for (int i = 0; i < MAX_PROCS; i++) {
        if (p_to_r[i][r]) {
            new_holder = i;
            break;
        }
    }
    if (new_holder != -1) {
        p_to_r[new_holder][r] = 0;
        r_holder[r] = new_holder;
        if (detect_and_print_cycle(new_holder)) {
            exit(-1);
        }
    }
}

int main() {
    init();
    int p, r;
    char action;
    while (scanf("%d %c %d", &p, &action, &r) == 3) {
        if (action == 'a') {
            handle_acquire(p, r);
        } else if (action == 'r') {
            handle_release(p, r);
        } else {
            fprintf(stderr, "Invalid action '%c'\n", action);
            return 1;
        }
    }
    return 0;
}
