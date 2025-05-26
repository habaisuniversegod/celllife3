#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>

#define FIELD_WIDTH 400
#define FIELD_HEIGHT 256

class Field;

enum CellState {
    S_NONE, S_DELETED, S_ORGANIC, S_ALIVE
};

enum Genes {
    G_NOTHING_FORWARD, G_FOOD_FORWARD, G_BRO_FORWARD, G_NOT_BRO_FORWARD, G_TRUE, G_FALSE, G_IS_ATTACKED, G_IS_COLLIDED, G_IS_GOT_ENERGY, G_NEXT
};

int main();

class Cell {
    friend Field;
    friend int main();

    uint_fast64_t genome[650];
    uint_fast64_t active_gene;
    uint_fast64_t pos_x;
    uint_fast64_t pos_y;
    int_fast64_t energy;
    int_fast64_t last_energy;
    int_fast64_t delta_energy;
    uint_fast64_t rotation;
    uint_fast64_t age;
    uint_fast64_t last_attacked_counter;
    uint_fast64_t last_reproducted_counter;
    uint_fast64_t is_attacked;
    uint_fast64_t is_collided_by_another;
    uint_fast64_t is_got_energy_by_another;
    int_fast64_t action_counter;
    uint_fast64_t organic_decay;
    uint_fast64_t organic_drift;

    CellState state;

    sf::Color color;
    double damage_absorption;
    double attack_efficiency;
    uint_fast64_t reproduction_threshold;
    uint_fast64_t max_energy;
    double mutation_prob;
    uint_fast64_t relation_threshold;
    double body_nutrition;
    double organics_efficiency;
    double photosynthesis_efficiency;

    void init_genes();
    void fill_genome();
    void fill_rand_genome();
    uint_fast64_t act(Field* field, uint_fast64_t gene_id);
    uint_fast64_t check_relationship(Cell* other);
    void force_reproduce(Field* field);
    void death(Field* field);
    void killing(Field* field);
    uint_fast64_t move(Field* field, uint_fast64_t parameter);
    uint_fast64_t rotate(Field* field, uint_fast64_t parameter);
    uint_fast64_t eat(Field* field, uint_fast64_t parameter);
    uint_fast64_t lookaround(Field* field, uint_fast64_t parameter);
    uint_fast64_t checkrels(Field* field, uint_fast64_t parameter);
    uint_fast64_t give_energy(Field* field, uint_fast64_t parameter);
    uint_fast64_t reproduct(Field* field, uint_fast64_t parameter);

    sf::Vector2u on_look(Field* field);

public:
    Cell();

    void init_random_cell(uint_fast64_t x, uint_fast64_t y);
    void init_child_cell(uint_fast64_t x, uint_fast64_t y, Cell* parent);
    //void init_from_genome(uint_fast64_t x, uint_fast64_t y, uint_fast64_t* genome);
    static void get_cell_color(Cell* cell, sf::Color* color);

    void step(Field* field);
    //void organic_step(Field* field);
    sf::Vector2i get_coords();
    uint_fast64_t is_alive();
};