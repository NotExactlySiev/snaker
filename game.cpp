#include <iostream>
#include <ncurses.h>
#include <vector>
#include <string>
#include <random>
#include <cstdlib>
#include <map>
using namespace std;

class Enemy;
class Item;
class Level;
class Game;

map<char, int> orders =
{
        {'q', 0}, //Exit
        {'h', 1}, //Go Left
        {'j', 2}, //Go Up
        {'k', 3}, //Go Down
        {'l', 4}, //Go Right
        {'t', 5}, //Pick Up Item
        {'c', 6} //Consume Item
};

enum tile_type { type_Empty, type_Enemy, type_Item, type_Obstacle };
string enemies_symbols = "@Z";
string obstacles_symbols = "#";
string items_symbols = "^";

class Level
{
private:
        vector<Enemy *> enemies;
        vector<Item *> items;
public:
        Level() {};
        int add_enemy(Enemy *);
        int add_item(Item *);
        int enemies_count() { return enemies.size(); }
        int items_count() { return items.size(); }
        int get_distance(Enemy *, Enemy *);
        Enemy * get_enemy(int);
        Enemy * get_hero() { return enemies[0]; }
        Enemy * get_tile_enemy(int, int);
        Item * get_item(int);
        Item * get_tile_item(int, int);
        tile_type get_tile_type(int, int);
        void kill_enemy(int);
        void update_enemies();
        void enemies_act();
};

class Game
{
private:
        Level * level;
        char raw_input;
public:
        Game() { level = new Level(); }
        Level * get_level() { return level; }
        char get_raw_input() { return raw_input; }
        void set_raw_input(char new_input) { raw_input = new_input; }
        void main_loop();
        void exterminate(int);
        void draw();
        Item * select_item(Enemy *);
};

class Enemy
{
private:
        //Index in Level::enemies
        int index;
        int pos_y;
        int pos_x;
        int hitpoint;
        int damage;
        Level * level_in;
        bool dead;
        char symbol;
        int order;
        vector<Item *> inv;
public:
        //Constructor
        Enemy(int y, int x, int hp, int dmg, char sym):
        pos_y(y), pos_x(x), hitpoint(hp), damage(dmg), symbol(sym)
        { dead = false; order = 0; }
        //Access
        int get_posy() { return pos_y; }
        int get_posx() { return pos_x; }
        int get_hp() { return hitpoint; }
        int get_dmg() { return damage; }
        int get_index() { return index; }
        char get_symbol() { return symbol; }
        Level * get_level() { return level_in; }
        bool has_order();
        int get_order() { return order; }
        bool is_dead() { return dead; }
        void set_posy(int new_y) { pos_y = new_y; }
        void set_posx(int new_x) { pos_x = new_x; }
        void set_hitpoint(int new_hp) { hitpoint = new_hp; }
        void set_index(int new_index) { index = new_index; }
        void give_order(int new_order) { order = new_order; }
        void set_level(Level * new_level) { level_in = new_level; }
        void kill() { dead = true; }
        //Action
        void walk(int);
        void act();
        void move(int, int);
        void attack(Enemy *);
        void take_damage(int);
        void take_order();
        void heal(int);
        void take_item();
        void consume_item(Item *);
        //Inventory Management
        void add_item(Item *);
        Item * get_item(int index) { return inv[index]; }
        int count_items() { return inv.size(); }
        //Misc
        int way_towards(Enemy *);
};

class Item
{
private:
        int durability;
        int pos_y, pos_x;
        int index;
        char symbol;
        Enemy * owner;
        bool equipable;
        bool is_equiped;
        map<string, int> properties;
        string display_name;
public:

        Item(int, int, map<string, int>, int, char, string);
        //Access
        int get_posy() { return pos_y; }
        int get_posx() { return pos_x; }
        int get_dur() { return durability; }
        int get_index() { return index; }
        char get_symbol() { return symbol; }
        Enemy * get_owner() { return owner; }
        string get_name() { return display_name; }
        void set_index(int new_index) { index = new_index; }
        void set_posy(int new_y) { pos_y = new_y; }
        void set_posx(int new_x) { pos_x = new_x; }
        void set_position(int new_y, int new_x) { set_posy(new_y); set_posx(new_x); }
        void set_dur(int new_dur) { durability = new_dur; }
        void set_owner(Enemy * new_owner) { owner = new_owner; }
        int get_property(string property) { return properties[property]; }
        //Properties
        bool is_consumable();
        //Action
        void consume();
};

int interpret(char);
void make_room(int, int, int, int);
void init()
{
        initscr();
        noecho();
        clear();
        curs_set(0);
}

Game * game = new Game();

//------------------------------------------------------------------------------
int main ()
{
        srand(2);
        int rng, gnr;
        map<string, int> potionp = {
                {"ConsumeHeal", 3},
        };
        //Initializing
        init();
        Enemy * renemy;
        Enemy * hero = new Enemy(15, 20, 20, 2, '@');
        Item * potion = new Item(16, 17, potionp, 3, '^', "Potion");
        game->get_level()->add_item(potion);
        game->get_level()->add_enemy(hero);
        for (int i = 0; i < 10; i++)
        {
                rng = rand() % 13 + 6;
                gnr = rand() % 38 + 17;
                renemy = new Enemy(rng, gnr, 4, 1, 'Z');
                game->get_level()->add_enemy(renemy);
        }
        game->main_loop();
}

void Game::main_loop ()
{
        Enemy * enemy;
        int order;
        draw();
        while (true)
        {
                set_raw_input(getch());
                if (interpret(get_raw_input()) == 0)
                {
                        exterminate(0);
                }
                for (int i = 0; i < get_level()->enemies_count(); i++)
                {
                        enemy = get_level()->get_enemy(i);
                        enemy->take_order();
                        enemy->act();
                        draw();
                }
                get_level()->update_enemies();
        }
}

void make_room (int y, int x, int width, int height)
{
        for (int i = 0; i < width; i++)
        {
                for (int j = 0; j < height; j++)
                {
                        if (i == 0 || j == 0 || i == width - 1 || j == height - 1)
                        {
                                mvaddch(i + y, j + x, '#');
                        }
                        else
                        {
                                mvaddch(i + y, j + x, '.');
                        }
                }
        }
}

Item::Item (int y, int x, map<string, int> props, int charge, char sym, string name):
        pos_y(y),
        pos_x(x),
        properties(props),
        durability(charge),
        symbol(sym),
        display_name(name)
{
        index = game->get_level()->items_count();
}

void Game::draw ()
{
        Enemy * enemy;
        Item * item;
        clear();
        mvprintw(0, 0, "Your Hitpoint is %d", get_level()->get_hero()->get_hp());
        make_room(5, 15, 15, 40);
        //Draw Items
        for (int i = 0; i < get_level()->items_count(); i++)
        {
                item = get_level()->get_item(i);
                mvaddch(item->get_posy(),
                        item->get_posx(),
                        item->get_symbol());
        }
        //Draw Enemies
        for (int i = 0; i < get_level()->enemies_count(); i++)
        {
                enemy = get_level()->get_enemy(i);
                mvaddch(enemy->get_posy(),
                        enemy->get_posx(),
                        enemy->get_symbol());
        }

}

Item * Game::select_item(Enemy * enemy)
{
        Item * item;
        int index;
        for(int i = 0; i < enemy->count_items(); i++)
        {
                item = enemy->get_item(i);
                mvprintw(i, 0, "%d - ", i);
                mvprintw(i, 4, item->get_name().c_str());
        }
        index = getch();
        if (index >= 0 && index < enemy->count_items())
        {
                return enemy->get_item(index);
        }
        else
        {
                //alert("This Item does not exits!");
        }

}

void Enemy::move (int new_y, int new_x)
{
        set_posy(new_y);
        set_posx(new_x);
}

void Enemy::walk (int direction)
{
        int new_y = pos_y;
        int new_x = pos_x;
        if (direction == 1)
        {
                new_x -= 1;
        }
        else if (direction == 2)
        {
                new_y += 1;
        }
        else if (direction == 3)
        {
                new_y -= 1;
        }
        else if (direction == 4)
        {
                new_x += 1;
        }

        char new_tile = mvinch(new_y, new_x);

        //Check the Destination Block to decide what action to do
        if (get_level()->get_tile_type(new_y, new_x) == type_Empty)
        {
                //Can move if there's nothing there
                move(new_y, new_x);
        }
        else if (get_level()->get_tile_type(new_y, new_x) == type_Item)
        {
                //Can walk on Items
                move(new_y, new_x);
                //Should pick it up if auto pick up is ON - TO-DO
        }
        else if (get_level()->get_tile_type(new_y, new_x) == type_Obstacle)
        {
                //Can't do anything if it's a wall
                return;
        }
        else if (get_level()->get_tile_type(new_y, new_x) == type_Enemy)
        {
                //Attack if enemy is on the way
                attack(game->get_level()->get_tile_enemy(new_y, new_x));
        }
}

void Enemy::consume_item (Item * item)
{
        if (!item->is_consumable())
        {
                //alert("That item is not consumable!");
                return;
        }

        item->consume();
}

//Reduces the Health and kills the enemy if it goes below zero
void Enemy::take_damage (int damage)
{
        hitpoint -= damage;
        if (hitpoint <= 0)
        {
                hitpoint = 0;
                kill();
        }
}

//Attacks and damages a target
void Enemy::attack (Enemy * target)
{
        if (target == NULL)
        {
                return;
        }
        target->take_damage(damage);
}

//Returns the appropriate "order" in order to get closer to the target
int Enemy::way_towards (Enemy * target)
{
        if (abs(target->get_posx() - pos_x) >= abs(target->get_posy() - pos_y))
        {
                if (target->get_posx() > pos_x)
                {
                        return 4;
                }
                else
                {
                        return 1;
                }
        }
        else
        {
                if (target->get_posy() > pos_y)
                {
                        return 2;
                }
                else
                {
                        return 3;
                }
        }
}

//Adds an enemy to the map and gives it an index
int Level::add_enemy (Enemy * enemy)
{
        int index;
        index = enemies.size();
        enemy->set_index(index);
        enemies.push_back(enemy);
        enemy->set_level(this);
        return index;
}

//Adds an item to the map and gives it an index
int Level::add_item (Item * item)
{
        int index = items.size();
        item->set_index(index);
        items.push_back(item);
        return index;
}

tile_type Level::get_tile_type(int y, int x)
{
        char tile = mvinch(y, x);
        tile_type type;
        if (tile == '.')
        {
                type = type_Empty;
        }
        else if (items_symbols.find(tile) != string::npos)
        {
                type = type_Item;
        }
        else if (obstacles_symbols.find(tile) != string::npos)
        {
                type = type_Obstacle;
        }
        else if (enemies_symbols.find(tile) != string::npos)
        {
                type = type_Enemy;
        }
        return type;
}

//Returns the enemy by index
Enemy * Level::get_enemy (int index)
{
        return enemies[index];
}

//Returns the enemy by position
Enemy * Level::get_tile_enemy (int y, int x)
{
        //Returns the pointer to whatever monster is in the called position
        //0 if there isn't any monster
        char tile = mvinch(y, x);
        Enemy * enemy;
        Enemy * result = NULL;
        if (tile == '#' || tile == '.')
        {
                return 0;
        }
        //Brute force
        for (int i = 0; i < enemies_count(); i++)
        {
                enemy = get_enemy(i);
                if (enemy->get_symbol() == tile)
                {
                        if (enemy->get_posy() == y && enemy->get_posx() == x)
                        {
                                result = enemy;
                        }
                }
        }
        return result;
}

//Returns the item by position
Item * Level::get_tile_item (int y, int x)
{
        //Returns the pointer to whatever item is in the called position
        //0 if there isn't any monster
        char tile = mvinch(y, x);
        Item * item;
        Item * result = NULL;
        //SMOKE WEED EVERYDAY
        if (tile == '#' || tile == '.')
        {
                return 0;
        }
        //Brute force
        for (int i = 0; i < items_count(); i++)
        {
                item = get_item(i);
                if (item->get_symbol() == tile)
                {
                        if (item->get_posy() == y && item->get_posx() == x)
                        {
                                result = item;
                        }
                }
        }
        return result;
}

void Level::update_enemies ()
{
        //Check if the player is dead
        Enemy * enemy = get_hero();
        if (enemy->is_dead())
        {
                game->exterminate(1);
        }
        //Check if any other monster is dead
        for (int i = 1; i < enemies_count(); i++)
        {
                enemy = get_enemy(i);
                if (enemy->is_dead())
                {
                        kill_enemy(i);
                }
        }
}

void Level::kill_enemy (int n)
{
        Enemy * enemy = get_enemy(n);
        delete enemy;
        enemies.erase(enemies.begin() + n);
}

int Level::get_distance (Enemy * enemy1, Enemy * enemy2)
{
        return (abs(enemy1->get_posx() - enemy2->get_posx())
              + abs(enemy1->get_posy() - enemy2->get_posy()));
}

Item * Level::get_item (int index)
{
        return items[index];
}

void Enemy::act ()
{
        Item * item;
        int order = get_order();
        if (has_order())
        {
                switch (order)
                {
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                                walk(order);
                                break;
                        case 5:
                                take_item();
                                break;
                        case 6:
                                item = game->select_item(this);
                                //consume_item(item);
                                break;
                }
                give_order(0);
        }
}

void Enemy::take_item ()
{
        Item * item;
        item = game->get_level()->get_tile_item(get_posy(), get_posx());
        if (item != NULL)
        {
                add_item(item);
        }
        else
        {
                //game->alert("Nothing Here!");
        }
}

bool Enemy::has_order ()
{
        if (order == 0)
        {
                return false;
        }
        return true;
}

//All Creature AI Happens in this function
void Enemy::take_order ()
{
        int order;
        //If it's the Hero, take the order from input
        if (get_index() == 0)
        {
                order = interpret(game->get_raw_input());
        }
        //Zombie AI
        else if (get_symbol() == 'Z')
        {
                if (game->get_level()->get_distance(this, game->get_level()->get_hero()) < 8)
                {
                        order = way_towards(game->get_level()->get_hero());
                }
        }
        give_order(order);
}

void Enemy::add_item (Item * item)
{
        inv.push_back(item);
        item->set_owner(this);
        item->set_position(-1, -1);
}

void Enemy::heal (int amount)
{
        set_hitpoint(get_hp() + amount);
        if (get_hp() > 20)
        {
                set_hitpoint(20);
        }
}

bool Item::is_consumable ()
{
        bool result = false;
        if (properties["Consumable"] != 0)
        {
                 result = true;
        }
        return false;
}

//Calculates and Activates every effect that the potion will have
void Item::consume ()
{
        if (durability > 0)
        {
                int heal = 0;
                heal += properties["ConsumeHeal"];
                get_owner()->heal(heal);
        }
}

int interpret (char raw_order)
{
        return orders[raw_order];
}

void Game::exterminate (int code)
{
        switch (code)
        {
                case 0:
                        clear();
                case 1:
                        clear();
                        printw("You Lost! Press any key to exit the game!");
                        getch();
                        clear();
        }
        endwin();
        exit(0);
}
