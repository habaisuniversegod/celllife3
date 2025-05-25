#pragma once

#include "cell.h"

#define FIELD_SIZE (FIELD_WIDTH * FIELD_HEIGHT)

#define FIELD_INDEX(x, y) ((y) * FIELD_WIDTH + (x))
// #define BREED_SIZE 8
// #define BREED_THRES 15
#define POP_SIZE 1

class Field {
    Cell* traverse_queue[FIELD_SIZE];
    Cell cells[FIELD_SIZE];
    Cell* cells_free_queue[FIELD_SIZE];
    Cell* field[FIELD_SIZE];
    
    uint_fast64_t tq_a, tq_b;
    uint_fast64_t cfq_a, cfq_b;
    Cell* c_allocator;

    
    Cell* allocate_cell();
    void remove_cell(uint64_t tq_idx);
    void remove_from_traverse(uint64_t tq_idx);
    
    public:
    
    uint_fast64_t coords_to_update[FIELD_SIZE];
    int_fast64_t updates_ptr;
    uint_fast64_t season_ctr;
    uint_fast64_t total_steps;
    uint_fast64_t cells_count;
    uint_fast64_t alive_count;
    uint_fast64_t max_alive_count;
    //uint_fast64_t generation;
    
    Field();
    void reinit();
    void add_to_traverse(Cell* cell);
    
    void create_first_generation(uint_fast64_t size);
    //void make_breeding();
    
    inline void add_coord_to_redraw(uint_fast64_t x, uint_fast64_t y){
        coords_to_update[++updates_ptr] = (y << 32) | x;
    }

    Cell* create_cell(uint_fast64_t x, uint_fast64_t y);
    int_fast64_t get_photo_energy(uint_fast64_t x, uint_fast64_t y);

    inline Cell *get_cell_by_coords(uint_fast64_t x, uint_fast64_t y)
    {
        return field[FIELD_INDEX(x, y)];
    }
    inline Cell *get_cell_by_idx(uint_fast64_t i)
    {
        return field[i];
    }
    inline void swap_cells_on_field(uint_fast64_t x1, uint_fast64_t y1, uint_fast64_t x2, uint_fast64_t y2)
    {
        Cell* tmp = field[FIELD_INDEX(x1, y1)];
        field[FIELD_INDEX(x1, y1)] = field[FIELD_INDEX(x2, y2)];
        field[FIELD_INDEX(x2, y2)] = tmp;
    }

    void simulate_step();
};