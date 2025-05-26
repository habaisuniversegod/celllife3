#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <fstream>
#include "field.h"
#include "cell.h"

#define SCALING 2

//sf::VertexArray field_gfx;
sf::Clock fps_clock;

uint32_t raw_field_gfx[FIELD_WIDTH * FIELD_HEIGHT];
sf::Texture field_gfx;
sf::Sprite field_gfx_sprite;


void init_field_gfx(){
    // field_gfx.resize(FIELD_WIDTH * FIELD_HEIGHT * 4);
    // field_gfx.setPrimitiveType(sf::Quads);
    // for (int x = 0; x < FIELD_WIDTH; x++)
    // {
    //     for (int y = 0; y < FIELD_HEIGHT; y++)
    //     {
    //         field_gfx[FIELD_INDEX(x, y) * 4] = sf::Vertex(sf::Vector2f(x * SCALING, y * SCALING));
    //         field_gfx[FIELD_INDEX(x, y) * 4 + 1] = sf::Vertex(sf::Vector2f(x * SCALING + SCALING, y * SCALING));
    //         field_gfx[FIELD_INDEX(x, y) * 4 + 2] = sf::Vertex(sf::Vector2f(x * SCALING + SCALING, y * SCALING + SCALING));
    //         field_gfx[FIELD_INDEX(x, y) * 4 + 3] = sf::Vertex(sf::Vector2f(x * SCALING, y * SCALING + SCALING));
    //     }  
    // } 
    field_gfx.create(FIELD_WIDTH, FIELD_HEIGHT);
    field_gfx_sprite.setTexture(field_gfx);
    field_gfx_sprite.setScale(sf::Vector2f(SCALING, SCALING));
}

// void init_pop(Field* field){
    //     srand(time(0));
    //     Cell* cell = field->create_cell(FIELD_WIDTH / 2, FIELD_HEIGHT / 2);
    //     cell->init_random_cell(FIELD_WIDTH / 2, FIELD_HEIGHT / 2);
    // }
    
    void render_field(Field* field){
        // for (uint_fast64_t i = 0; i < FIELD_SIZE * 4;)
        // {
            //     Cell* cl = field->get_cell_by_idx(i >> 2);
            //     Cell::get_cell_color(cl, &field_gfx[i++].color);
            //     Cell::get_cell_color(cl, &field_gfx[i++].color);
            //     Cell::get_cell_color(cl, &field_gfx[i++].color);
            //     Cell::get_cell_color(cl, &field_gfx[i++].color);
            // }
            
            // while (field->updates_ptr >= 0)
            // {
                //     uint_fast64_t coord = field->coords_to_update[field->updates_ptr--];
                //     Cell* cl = field->get_cell_by_coords(coord & 0xffffffff, coord >> 32);
                //     cl->get_cell_color(cl, &field_gfx[((coord >> 32) * FIELD_WIDTH + (coord & 0xffffffff)) * 4].color);    
                //     cl->get_cell_color(cl, &field_gfx[((coord >> 32) * FIELD_WIDTH + (coord & 0xffffffff)) * 4 + 1].color);    
                //     cl->get_cell_color(cl, &field_gfx[((coord >> 32) * FIELD_WIDTH + (coord & 0xffffffff)) * 4 + 2].color);    
                //     cl->get_cell_color(cl, &field_gfx[((coord >> 32) * FIELD_WIDTH + (coord & 0xffffffff)) * 4 + 3].color);    
                // }
                
                while (field->updates_ptr >= 0)
                {
                    uint_fast64_t coord = field->coords_to_update[field->updates_ptr--];
                    Cell* cl = field->get_cell_by_coords(coord & 0xffffffff, coord >> 32);
                    Cell::get_cell_color(cl, (sf::Color*)&raw_field_gfx[FIELD_INDEX(coord & 0xffffffff, coord >> 32)]);
                }
                field_gfx.update((uint8_t*)raw_field_gfx);
            }
            
            char stat_buffer[2048];

void draw_selector(sf::Vector2i coords, sf::RenderTarget& target){
    static sf::RectangleShape vl, hl;
    vl.setFillColor(sf::Color::White);
    vl.setSize(sf::Vector2f(1, FIELD_HEIGHT * SCALING));
    hl.setFillColor(sf::Color::White);
    hl.setSize(sf::Vector2f(FIELD_WIDTH * SCALING, 1));
    vl.setPosition(coords.x, 0);
    hl.setPosition(0, coords.y);
    if (coords.x < FIELD_WIDTH * SCALING && coords.y < FIELD_HEIGHT * SCALING){
        target.draw(vl);
        target.draw(hl);
    }
}
            
int main(){
    srand(time(0));
    Field* field = new Field();
    
    init_field_gfx();
    field->create_first_generation(POP_SIZE);
    
    sf::RenderWindow win(sf::VideoMode(FIELD_WIDTH * SCALING + 250, FIELD_HEIGHT * SCALING + 96), "CellLife 3.1.1");
    
    sf::Font font;
    font.loadFromFile("font.ttf");
                
    sf::Text stats;
    stats.setFillColor(sf::Color::White);
    stats.setFont(font);
    stats.setPosition(2, FIELD_HEIGHT * SCALING + 2);
    stats.setCharacterSize(14);

    sf::Text cell_info;
    cell_info.setFillColor(sf::Color::White);
    cell_info.setFont(font);
    cell_info.setPosition(FIELD_WIDTH * SCALING + 2, 2);
    cell_info.setCharacterSize(10);

    //std::ofstream gen_stats("generations-history.txt", std::ios::out);

    bool is_running = true;
    bool is_pause = false;

    sf::Vector2i mouse;
    Cell* selected_cell;
    bool is_cell_selected = false;
    bool blinker_flag = true;

    uint_fast64_t last_micros = 0;
    uint_fast64_t micros = 0;
    int fps = 0;
    fps_clock.restart();

    while (is_running){
        blinker_flag = !blinker_flag;
        mouse = sf::Mouse::getPosition(win);
        sf::Event event;
        while (win.pollEvent(event)){
            if (event.type == sf::Event::Closed){
                is_running = false;
            }
            else if (event.type == sf::Event::KeyPressed){
                if (event.key.code == sf::Keyboard::R){
                    goto reinit_all;
                }
                if (event.key.code == sf::Keyboard::P){
                    is_pause = !is_pause;
                }
                if (event.key.code == sf::Keyboard::Space){
                    field->simulate_step();
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased){
                if (event.mouseButton.button == sf::Mouse::Button::Left && mouse.x < FIELD_WIDTH * SCALING && mouse.y < FIELD_HEIGHT * SCALING){
                    is_cell_selected = false;
                    selected_cell = field->get_cell_by_coords(mouse.x / SCALING, mouse.y / SCALING);
                    if (selected_cell == nullptr) continue;
                    if (!selected_cell->is_alive()) continue;
                    is_cell_selected = true;
                }
                else {
                    is_cell_selected = false;
                }
            }
        }
        snprintf(stat_buffer, 2048, "Iters: %llu\nPopulation(cur/max): %llu/%llu\nSeason counter/Light: %llu/%llu\nTPS: %d\nField() memory usage: %.2lf MiB",
        field->total_steps, field->alive_count, field->max_alive_count, field->season_ctr, field->get_photo_energy(FIELD_WIDTH / 2, FIELD_HEIGHT / 2),
        fps, sizeof(Field) / (1024.0 * 1024.0));
        stats.setString(stat_buffer);

        win.clear(sf::Color::Black);
        render_field(field);
        win.draw(field_gfx_sprite);
        win.draw(stats);

        if (!is_cell_selected){
            draw_selector(mouse, win);
        }else {
            if (!selected_cell->is_alive()){
                is_cell_selected = false;
            }
            else if (blinker_flag){
                draw_selector(selected_cell->get_coords() * SCALING, win);
            }
            for (uint_fast64_t i = 0; i < 65; i++)
            {
                cell_info.setPosition(FIELD_WIDTH * SCALING + 5, 2 + 8 * i + (i == 64 ? 5 : 0));
                cell_info.setFillColor(selected_cell->active_gene == i ? sf::Color::Yellow : sf::Color::White);
                snprintf(stat_buffer, 2048, "%llu | %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu ",
                i, selected_cell->genome[i * 10],
                selected_cell->genome[i * 10 + 1],
                selected_cell->genome[i * 10 + 2],
                selected_cell->genome[i * 10 + 3],
                selected_cell->genome[i * 10 + 4],
                selected_cell->genome[i * 10 + 5],
                selected_cell->genome[i * 10 + 6],
                selected_cell->genome[i * 10 + 7],
                selected_cell->genome[i * 10 + 8],
                selected_cell->genome[i * 10 + 9]);
                cell_info.setString(stat_buffer);
                win.draw(cell_info);
            }
            cell_info.setPosition(FIELD_WIDTH / 2 * SCALING + 2, FIELD_HEIGHT * SCALING + 2);
            snprintf(stat_buffer, 2048, "Energy/Max Energy: %llu/%llu\nRepr.Thres. %llu\nActive gene: %llu",
            selected_cell->energy, selected_cell->max_energy, selected_cell->reproduction_threshold, selected_cell->active_gene);
            cell_info.setString(stat_buffer);
            win.draw(cell_info);
        }

        win.display();

        micros = fps_clock.getElapsedTime().asMicroseconds();
        fps = 1e6 / (micros - last_micros);
        last_micros = micros;
        
        if (is_pause) continue;

        field->simulate_step();

        if (field->alive_count == 0){
            reinit_all:
            //gen_stats << field->generation << " " << field->total_steps << " " << field->max_alive_count << "\n";
            field->reinit();
            field->create_first_generation(POP_SIZE);
        }
    }
    delete field;
    //gen_stats.close();
    return 0;
}