#include "cell.h"
#include "field.h"
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>

#define GENE_RANGE(x) (genome[640 + (x)] / 63.0)
#define TO_RANGE(x, a, b) ((a) + ((b) - (a)) * (x))

sf::Color hsv2rgb(double h, double s, double v) {
    // Ограничиваем значения в допустимых пределах [0, 1]
    h = std::fmod(h, 1.0); // H ∈ [0, 1)
    if (h < 0.0) h += 1.0; // Если H был отрицательным

    // Преобразуем H в диапазон [0, 6)
    double h_scaled = h * 6.0;
    int sector = static_cast<int>(std::floor(h_scaled));
    double fraction = h_scaled - sector;

    // Промежуточные значения
    double p = v * (1.0 - s);
    double q = v * (1.0 - s * fraction);
    double t = v * (1.0 - s * (1.0 - fraction));

    double r, g, b;

    switch (sector % 6) {
        case 0: r = v; g = t; b = p; break; // Красный → Жёлтый
        case 1: r = q; g = v; b = p; break; // Жёлтый → Зелёный
        case 2: r = p; g = v; b = t; break; // Зелёный → Голубой
        case 3: r = p; g = q; b = v; break; // Голубой → Синий
        case 4: r = t; g = p; b = v; break; // Синий → Пурпурный
        case 5: r = v; g = p; b = q; break; // Пурпурный → Красный
        default: r = g = b = 0.0; break; // Не должно произойти
    }

    // Преобразуем в sf::Color (автоматически приводит к uint8_t)
    return sf::Color(
        static_cast<uint8_t>(std::round(r * 255.0)),
        static_cast<uint8_t>(std::round(g * 255.0)),
        static_cast<uint8_t>(std::round(b * 255.0))
    );
}

void Cell::init_genes()
{
    color = hsv2rgb(GENE_RANGE(0), 0.66, 0.85);
    damage_absorption = GENE_RANGE(1) * 0.33;
    attack_efficiency = TO_RANGE(GENE_RANGE(2), 1.0, 1.25);
    reproduction_threshold = TO_RANGE(GENE_RANGE(3), 0.66, 0.9);
    max_energy = TO_RANGE(GENE_RANGE(4), 1000, 1750);
    mutation_prob = TO_RANGE(GENE_RANGE(5), 0.1, 0.5);
    relation_threshold = TO_RANGE(GENE_RANGE(6), 600, 650);
    body_nutrition = TO_RANGE(GENE_RANGE(7), 0.0075, 0.03);
    organics_efficiency = TO_RANGE(GENE_RANGE(8), 0.75, 1.0);
    photosynthesis_efficiency = TO_RANGE(GENE_RANGE(9), 0.6, 1.0);
}

void Cell::fill_genome()
{
    for (uint_fast64_t i = 0; i < 640; i++)
    {
        genome[i] = 56;
    }
    for (uint_fast64_t i = 640; i < 650; i++)
    {
        genome[i] = rand() % 64;
    }
    
}

void Cell::fill_rand_genome()
{
    for (uint_fast64_t i = 0; i < 650; i++)
    {
        genome[i] = rand() % 64;
    }
}

uint_fast64_t Cell::act(Field* field, uint_fast64_t gene_id)
{
    uint_fast64_t result = 0;
    uint_fast64_t group = genome[active_gene * 10 + gene_id];
    uint_fast64_t subaction = group % 8;
    group >>= 3;
    //printf("C%llu,%llu: energy: %llu, action: %llu, gene: %llu,%llu\n", pos_x, pos_y, energy, genome[active_gene * 10 + gene_id], active_gene, gene_id);

    switch(group){
        case 0:
        //printf("move\n");
        return move(field, subaction);
        case 1:
        //printf("rotate\n");
        return rotate(field, subaction);
        case 2:
        //printf("eat\n");
        return eat(field, subaction);
        case 3:
        //printf("lookaround\n");
        return lookaround(field, subaction);
        case 4:
        //printf("checkrels\n");
        return checkrels(field, subaction);
        case 5:
        //printf("give\n");
        return give_energy(field, subaction);
        case 6:
        //printf("repr\n");
            return reproduct(field, subaction);
        case 7:
            switch (subaction)
            {
                case 0:
                    energy += (result = field->get_photo_energy(pos_x, pos_y));
                    return result;
                case 1:
                    return energy >= reproduction_threshold;
                case 2:
                    return energy == max_energy;
                case 3:
                    return last_attacked_counter < 50;
                case 4:
                    return last_reproducted_counter < 100;
                case 5:
                    return delta_energy > 0;
                case 6:
                    return age < 100;
                case 7:
                    active_gene = (active_gene + 1) % 64;
                    action_counter = 0;
                    return 1;
            }
            action_counter--;
    }
    return 0;
}

sf::Vector2u Cell::on_look(Field* field){
    static const sf::Vector2u directions[8] = {{0, 1}, {1, 1}, {1, 0}, {1, FIELD_HEIGHT-1}, {0, FIELD_HEIGHT-1}, {FIELD_WIDTH-1, FIELD_HEIGHT-1}, {FIELD_WIDTH-1, 0}, {FIELD_WIDTH-1, 1}};
    return sf::Vector2u((pos_x + directions[rotation].x) % FIELD_WIDTH, (pos_y + directions[rotation].y) % FIELD_HEIGHT);
}

void Cell::death(Field* field)
{
    state = S_ORGANIC;
    field->add_coord_to_redraw(pos_x, pos_y);
    //organic_decay = 500 + rand() % (5000 - 500);
    //organic_drift = 100 + rand() % (250 - 100);
}

void Cell::killing(Field *field)
{
    state = S_DELETED;
    field->add_coord_to_redraw(pos_x, pos_y);
}

uint_fast64_t Cell::move(Field *field, uint_fast64_t parameter)
{
    uint_fast64_t old_rot = rotation;
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    Cell* cl = field->get_cell_by_coords(npos.x, npos.y);
    rotation = old_rot;
    action_counter = 0;
    if (cl == nullptr){
        //printf("trying to move from %llu,%llu to %llu,%llu\n, cell in: %p\n",
        //pos_x, pos_y, npos.x, npos.y );
        field->swap_cells_on_field(pos_x, pos_y, npos.x, npos.y);
        field->add_coord_to_redraw(pos_x, pos_y);
        field->add_coord_to_redraw(npos.x, npos.y);
        pos_x = npos.x;
        pos_y = npos.y;
        energy--;
        return 1;
    }
    cl->is_collided_by_another = 1;
    return 0;
}

uint_fast64_t Cell::rotate(Field *field, uint_fast64_t parameter)
{
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    action_counter -= 2;
    return field->get_cell_by_coords(npos.x, npos.y) != nullptr;
}

uint_fast64_t Cell::eat(Field *field, uint_fast64_t parameter)
{
    uint_fast64_t attack_energy = 0;
    uint_fast64_t old_rot = rotation;
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    Cell* cl = field->get_cell_by_coords(npos.x, npos.y);
    rotation = old_rot;
    action_counter = 0;
    if (cl != nullptr){
        if (cl->state == S_ORGANIC){
            energy += cl->max_energy * cl->body_nutrition * organics_efficiency;
            field->add_to_traverse(cl);
            cl->killing(field);
        }
        else {
            attack_energy = (cl->max_energy * 0.2) / (1 + cl->damage_absorption) * attack_efficiency;
            if (attack_energy > cl->energy){
                attack_energy = cl->energy;
            }
            energy += attack_energy;
            cl->energy -= attack_energy;
            cl->is_attacked = 1;
        }
        return 1;
    }
    energy -= 2;
    return 0;
}

uint_fast64_t Cell::lookaround(Field *field, uint_fast64_t parameter)
{
    uint_fast64_t old_rot = rotation;
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    rotation = old_rot;
    action_counter--;
    return field->get_cell_by_coords(npos.x, npos.y) != nullptr;
}

uint_fast64_t Cell::checkrels(Field *field, uint_fast64_t parameter)
{
    uint_fast64_t old_rot = rotation;
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    rotation = old_rot;
    action_counter--;
    Cell* cl = field->get_cell_by_coords(npos.x, npos.y);
    if (cl == nullptr){
        return 0;
    }
    return check_relationship(cl) && (cl->state == S_ALIVE);
}

uint_fast64_t Cell::give_energy(Field *field, uint_fast64_t parameter)
{
    uint_fast64_t given = 0;
    uint_fast64_t old_rot = rotation;
    rotation = (rotation + parameter) % 8;
    sf::Vector2u npos = on_look(field);
    rotation = old_rot;
    action_counter = 0;
    Cell* cl = field->get_cell_by_coords(npos.x, npos.y);
    if (cl != nullptr){
        given = (energy * 0.05) * (cl->state == S_ALIVE);
        energy -= given;
        cl->energy += given;
        cl->is_got_energy_by_another = 1;
        return given > 0;
    }
    return 0;
}

uint_fast64_t Cell::reproduct(Field *field, uint_fast64_t parameter)
{
    if (energy < reproduction_threshold){
        return 0;
    }
    uint_fast64_t old_rot = rotation;
    uint_fast64_t rot_end = (rotation + parameter) % 8;
    rotation = rot_end;
    action_counter = 0;
    last_reproducted_counter = 0;
    do {
        sf::Vector2u npos = on_look(field);
        Cell* cell = field->get_cell_by_coords(npos.x, npos.y);
        if (cell == nullptr){
            Cell* cl = field->create_cell(npos.x, npos.y);
            cl->init_child_cell(npos.x, npos.y, this);
            field->add_coord_to_redraw(npos.x, npos.y);
            rotation = old_rot;
            return 1;
        }
        cell->is_collided_by_another = 1;
    } while ((rotation = (rotation + 1) % 8) != rot_end);
    death(field);
    return 0;
}

uint_fast64_t Cell::check_relationship(Cell *other)
{
    uint_fast64_t common_genes = 0;
    for (uint_fast64_t i = 0; i < 650; i++)
    {
        common_genes += genome[i] == other->genome[i];
    }
    return common_genes >= relation_threshold;
}

void Cell::force_reproduce(Field* field)
{
    uint_fast64_t old_rot = rotation;
    action_counter = 0;
    last_reproducted_counter = 0;
    do {
        sf::Vector2u npos = on_look(field);
        Cell* cell = field->get_cell_by_coords(npos.x, npos.y);
        if (cell == nullptr){
            Cell* cl = field->create_cell(npos.x, npos.y);
            cl->init_child_cell(npos.x, npos.y, this);
            field->add_coord_to_redraw(npos.x, npos.y);
            return;
        }
        cell->is_collided_by_another = 1;
    } while ((rotation = (rotation + 1) % 8) != old_rot);
    death(field);
}

Cell::Cell()
{
}

void Cell::init_random_cell(uint_fast64_t x, uint_fast64_t y)
{
    fill_genome();
    init_genes();
    state = S_ALIVE;
    active_gene = 0;
    pos_x = x;
    pos_y = y;
    age = 0;
    energy = 250;
    rotation = std::rand() % 8;
    last_attacked_counter = 0;
    last_reproducted_counter = 0;
}

void Cell::init_child_cell(uint_fast64_t x, uint_fast64_t y, Cell* parent)
{
    memcpy(genome, parent->genome, 650 * sizeof(uint_fast64_t));
    if (rand() / (double)RAND_MAX < parent->mutation_prob){
        uint_fast64_t rg = rand() % 650;
        genome[rg] = rand() % 64;
        //printf("MUTATION (%lf): gene[%llu]: %llu -> %llu\n", parent->mutation_prob, rg, parent->genome[rg], genome[rg]);
    }
    init_genes();
    state = S_ALIVE;
    active_gene = rand() % 64;
    pos_x = x;
    pos_y = y;
    age = 0;
    parent->energy /= 2;
    energy = parent->energy * 2 / 3; //66%
    rotation = parent->rotation;
    last_attacked_counter = 0;
    last_reproducted_counter = 0;
}

// void Cell::init_from_genome(uint_fast64_t x, uint_fast64_t y, uint_fast64_t *genome)
// {
//     memcpy(this->genome, genome, 650 * sizeof(uint_fast64_t));
//     init_genes();
//     state = S_ALIVE;
//     active_gene = 0;
//     pos_x = x;
//     pos_y = y;
//     age = 0;
//     energy = 250;
//     rotation = std::rand() % 8;
//     last_attacked_counter = 0;
//     last_reproducted_counter = 0;
// }

void Cell::get_cell_color(Cell* cell, sf::Color* color)
{
    static sf::Color colors[4] = {sf::Color(15, 15, 15), sf::Color(15, 15, 15), sf::Color(55, 55, 55), sf::Color::White};
    if (cell == nullptr){
        *color = sf::Color(15, 15, 15);
        return;
    }
    colors[3] = cell->color;
    *color = colors[cell->state];
}

void Cell::step(Field* field)
{
    action_counter = state == S_ALIVE ? 8 : 0;

    if (energy >= max_energy && action_counter > 0){
        energy = max_energy;
        force_reproduce(field);
    }
    if (energy <= 0){
        if (is_attacked){
            killing(field);
            return;
        }
        death(field);
        return;
    }

    last_energy = energy;

    if (is_attacked && action_counter > 0){
        act(field, G_IS_ATTACKED);
        is_attacked = 0;
        last_attacked_counter = 0;
    }
    if (is_collided_by_another && action_counter > 0){
        is_collided_by_another = 0;
        act(field, G_IS_COLLIDED);
    }
    if (is_got_energy_by_another && action_counter > 0){
        is_got_energy_by_another = 0;
        act(field, G_IS_GOT_ENERGY);
    }
    
    uint_fast64_t decision = 0;
    sf::Vector2u forward_pos = on_look(field);
    Cell* forward = field->get_cell_by_coords(forward_pos.x, forward_pos.y);
    CellState forward_state = forward == nullptr ? S_NONE : forward->state;

    switch (forward_state)
    {
    case S_ORGANIC:
        decision = 1;
        break;
    case S_ALIVE:
        decision = 2 + check_relationship(forward);
        break;
    default:
        break;
    }
    if (action_counter <= 0) goto finalize_cell_step;
    uint_fast64_t condition;
    condition = act(field, decision) != 0;

    if (action_counter <= 0) goto finalize_cell_step;
    act(field, G_TRUE + condition);

    active_gene = genome[active_gene * 10 + G_NEXT];

    finalize_cell_step:
    age++;
    last_reproducted_counter++;
    last_attacked_counter++;
    energy--;
    delta_energy = energy - last_energy;
}
