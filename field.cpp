#include "field.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

int_fast64_t abs_(int_fast64_t x){
    return x < 0 ? -x : x;
}

int_fast64_t max(int_fast64_t a, int_fast64_t b){
    return a > b ? a : b;
}

Cell* Field::allocate_cell()
{
    //printf("alloc/cfq_a/b: %llu/%llu/%llu\n", uint_fast64_t(c_allocator - cells), cfq_a, cfq_b);
    if (cfq_a == cfq_b){
        return c_allocator++;
    }
    Cell* result = cells_free_queue[cfq_a++];
    cfq_a %= FIELD_SIZE;
    return result;
}

void Field::remove_from_traverse(uint64_t tq_idx)
{
    traverse_queue[tq_idx] = traverse_queue[tq_a];
    traverse_queue[tq_a++] = nullptr;
    tq_a %= FIELD_SIZE;
}

void Field::remove_cell(uint_fast64_t tq_idx) {
    Cell* cl = traverse_queue[tq_idx];
    cl->state = S_NONE;
    cells_free_queue[cfq_b++] = cl;
    cfq_b %= FIELD_SIZE;
    field[FIELD_INDEX(cl->pos_x, cl->pos_y)] = nullptr;
    remove_from_traverse(tq_idx);
    cells_count--;
};

Field::Field()
{
    reinit();
}

void Field::reinit()
{
    cells_count = 0;
    max_alive_count = 0;
    c_allocator = &cells[0];
    tq_a = tq_b = 0;
    cfq_a = cfq_b = 0;
    updates_ptr = FIELD_SIZE - 1;
    season_ctr = alive_count = total_steps = 0;
    for (uint_fast64_t i = 0; i < FIELD_SIZE; i++)
    {
        field[i] = nullptr;
        traverse_queue[i] = nullptr;
        cells[i].state = S_NONE;
        coords_to_update[i] = ((i / FIELD_WIDTH) << 32) | (i % FIELD_WIDTH);
    }
}

void Field::add_to_traverse(Cell *cell)
{
    traverse_queue[tq_b++] = cell;
    tq_b %= FIELD_SIZE;
}

void Field::create_first_generation(uint_fast64_t size)
{
    while (size > 0)
    {
        int rx = rand() % FIELD_WIDTH;
        int ry = rand() % FIELD_HEIGHT;
        if (field[FIELD_INDEX(rx, ry)] != nullptr){
            continue;
        }
        Cell* cell = create_cell(rx, ry);
        cell->init_random_cell(rx, ry);
        size--;
    }
}

// void Field::make_breeding()
// {
//     uint_fast64_t* candidates[BREED_SIZE];
//     uint_fast64_t gene_buffer[10];
//     uint_fast64_t cand_ptr = 0;
    
//     if (!cells_count) return;
    
//     uint_fast64_t i = tq_a;
//     do {
//         Cell* cl = traverse_queue[i % FIELD_SIZE];
//         candidates[cand_ptr] = new uint_fast64_t[650];
//         memcpy(candidates[cand_ptr], cl->genome, 650 * sizeof(uint_fast64_t));
//         cand_ptr++;
//     } while (++i % FIELD_SIZE != tq_b && cand_ptr < BREED_SIZE);
    
//     if (cand_ptr != BREED_SIZE) return;

//     reinit();
//     for (i = 0; i < BREED_SIZE; i++)
//     {
//         for (int i = 0; i < BREED_SIZE / 2; i++)
//         {
//             uint_fast64_t* g1 = &candidates[rand() % BREED_SIZE][(rand() % 65) * 10];
//             uint_fast64_t* g2 = &candidates[i][(rand() % 65) * 10];
//             memcpy(gene_buffer, g1, 10);
//             memcpy(g1, g2, 10);
//             memcpy(g1, gene_buffer, 10);
//         }
//         candidates[i][rand() % 650] = rand() % 64;

//         for (uint_fast64_t j = 0; j < POP_SIZE / BREED_SIZE; j++)
//         {
//             int rx = rand() % FIELD_WIDTH;
//             int ry = rand() % FIELD_HEIGHT;
//             if (field[FIELD_INDEX(rx, ry)] != nullptr){
//                 continue;
//             }
//             Cell* cell = create_cell(rx, ry);
//             cell->init_from_genome(rx, ry, candidates[i]);
//         }
//     }
//     for (i = 0; i < BREED_SIZE; i++)
//     {
//         delete[] candidates[i];
//     } 
//     generation++;
// }

Cell* Field::create_cell(uint_fast64_t x, uint_fast64_t y)
{
    //printf("tq_a/b: %llu/%llu\n", tq_a, tq_b);
    Cell* cl = allocate_cell();
    traverse_queue[tq_b++] = cl;
    tq_b %= FIELD_SIZE;
    field[FIELD_INDEX(x, y)] = cl;
    cells_count++;
    return cl;
}

int_fast64_t Field::get_photo_energy(uint_fast64_t x, uint_fast64_t y)
{
    static int_fast64_t photo_energy_by_season[16] = 
    {12, 12, 11, 10, 9, 8, 7, 6, 6, 6, 7, 8, 9, 10, 11, 12};
    int_fast64_t season_max = photo_energy_by_season[season_ctr / 2500];
    int_fast64_t distance = max(abs_(FIELD_WIDTH / 2 - x), abs_(FIELD_HEIGHT / 2 - y));
    int_fast64_t energy = (FIELD_WIDTH * 7 / 16 - distance) / 5;
    return energy > season_max ? season_max : (energy < season_max ? 0 : energy);
}

void Field::simulate_step()
{
    season_ctr = (season_ctr + 1) % 40000;
    total_steps++;
    if (!cells_count) return;
    uint_fast64_t i = tq_a;
    alive_count = 0;
    do {
        Cell* cl = traverse_queue[i % FIELD_SIZE];
        cl->step(this);
        if (cl->state == S_DELETED){
            remove_cell(i % FIELD_SIZE);
        }
        else if (cl->state == S_ORGANIC){
            remove_from_traverse(i % FIELD_SIZE);
        }
        alive_count += cl->state == S_ALIVE;
        max_alive_count = alive_count > max_alive_count ? alive_count : max_alive_count;
    } while (++i % FIELD_SIZE != tq_b);
}
