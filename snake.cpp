#include <SFML/Graphics.hpp>
#include <list>
#include <iostream>
#include <random>
#include <ctime>
#include <sstream>

#include <Windows.h>
#include <MMSystem.h>

using namespace std;

typedef unsigned long long llu;

const int windowWidth {700}, windowHeight {500};
const int brickWidth {20}, brickHeight {20};
enum {U,R,D,L};

bool music;

//TODO: tytulowe okno
//TODO: trudnosc
//TODO: wyniki
//TODO: szybkie zawracanie
//TODO: mute
//TODO: przyciski aktywne okno

int difficulty;
// easy, normal, hard

string imgCatalog = "files/img/";
string soundsCatalog = "files/sounds/";

float snakesSpeedBegin = 0.07;
float snakesSpeed = snakesSpeedBegin;

int startLives = 3;

void * snakesW = NULL;

sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SNAKE (sfml) P. Gardziejewski", sf::Style::Close);

template < class T >
string to_string( T t )
{
    stringstream ss;
    ss << t;
    return ss.str();
}

class TextureManager
{
private:
    map<string, sf::Texture> textures;

public:
    void loadTexture(const string& name, const string &filename);
    sf::Texture& getRef(const string& texture);
    TextureManager()
    {
    }
};

void TextureManager::loadTexture(const string& name, const string& filename)
{
    sf::Texture tex;
    tex.loadFromFile(filename);
    this->textures[name] = tex;
    return;
}

sf::Texture& TextureManager::getRef(const string& texture)
{
    return this->textures.at(texture);
}

TextureManager texmgr;

class ShapeB
{
public:
    sf::Vector2f pos;
    sf::RectangleShape shape;
};

class WallB : public ShapeB
{
public:
    WallB(sf::Vector2f v)
    {
        pos.x = v.x;
        pos.y = v.y;
        shape.setPosition(v.x+1, v.y+1);
        shape.setSize({brickWidth-2, brickHeight-2});
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin(0,0);
    }
};

class SnakeB : public ShapeB
{
public:
    SnakeB(sf::Vector2f v)
    {
        pos.x = v.x;
        pos.y = v.y;
        shape.setPosition(v.x+1, v.y+1);
        shape.setSize({brickWidth-2, brickHeight-2});
        shape.setFillColor(sf::Color::Yellow);
        shape.setOrigin(0,0);
    }
};


class SpecB : public ShapeB
{
public:
    int type;
    int points;
    sf::Sprite spec_sprite;

    SpecB(sf::Vector2f v, int t)
    {
        type = t;
        pos.x = v.x;
        pos.y = v.y;
        if (t == 0)
        {
            spec_sprite.setTexture(texmgr.getRef("clock"));
        }
        else if (t == 1)
        {
            spec_sprite.setTexture(texmgr.getRef("scissors"));
        }
        else if (t == 2)
        {
            spec_sprite.setTexture(texmgr.getRef("extraLive"));
        }

        spec_sprite.setPosition(v.x, v.y);
        spec_sprite.setOrigin(0,0);
    }
};

class FoodB : public ShapeB
{
public:
    int type;
    int points;
    sf::Sprite food_sprite;

    FoodB(sf::Vector2f v, int t)
    {
        type = t;
        pos.x = v.x;
        pos.y = v.y;
        if (t == 0)
        {
            food_sprite.setTexture(texmgr.getRef("apple"));
            points = 1;
        }
        else if (t == 1)
        {
            food_sprite.setTexture(texmgr.getRef("banana"));
            points = 2;
        }
        else if (t == 2)
        {
            food_sprite.setTexture(texmgr.getRef("strawberry"));
            points = 1;
        }
        else if (t == 3)
        {
            food_sprite.setTexture(texmgr.getRef("pear"));
            points = 3;
        }
        else if (t == 4)
        {
            food_sprite.setTexture(texmgr.getRef("watermelon"));
            points = 5;
        }
        else if (t == 5)
        {
            food_sprite.setTexture(texmgr.getRef("grape"));
            points = 3;
        }
        else if (t == 6)
        {
            food_sprite.setTexture(texmgr.getRef("cherry"));
            points = 3;
        }


        food_sprite.setPosition(v.x, v.y);
        food_sprite.setOrigin(0,0);
    }
};

list <WallB> wall;
list <FoodB> food;
list <SpecB> spec;

class Wall
{
public:
    Wall()
    {
        for (int i = 0; i < windowWidth; i+=brickWidth)
        {
            wall.emplace_back(sf::Vector2f(i, 0));
            wall.emplace_back(sf::Vector2f(i, windowHeight-brickHeight));
        }
        for (int i = brickHeight; i < windowHeight; i+=brickHeight)
        {
            wall.emplace_back(sf::Vector2f(0, i));
            wall.emplace_back(sf::Vector2f(windowWidth-brickWidth, i));
        }
        update ();
    }

    void update ()
    {
        for(auto& w : wall) window.draw(w.shape);
    }

};

class Snake
{
    int startLength = 5;
    sf::Vector2f curr, startPosition;
    list <SnakeB> tail;
    int length = startLength;
    short direction = L;

public:

    int lives;
    int points;
    int pointsMulti;
    int foodEaten;

    Snake ()
    {
        lives = startLives;
        pointsMulti = 1;
        foodEaten = 0;
        points = 0;
        tail.clear();
        startPosition = {(int)floor((windowWidth/2*3) / (brickWidth * 2)) * brickWidth, (int)floor(windowHeight / (brickHeight * 2)) * brickHeight};
        curr = startPosition;
        length = startLength;
        direction = L;
        tail.emplace_back(curr);
        update ();
    }

    void reset ()
    {
        snakesSpeed = snakesSpeedBegin;
        pointsMulti = 1;
        foodEaten = 0;
        tail.clear();
        curr = startPosition;
        length = startLength;
        direction = L;
    }

    void update ()
    {
        for(auto& s : tail) window.draw(s.shape);
    }

    sf::Vector2f move ()
    {
        if (direction == L)
        {
            curr.x -= brickWidth;
            tail.emplace_back(curr);
        }
        else if (direction == R)
        {
            curr.x += brickWidth;
            tail.emplace_back(curr);
        }
        else if (direction == U)
        {
            curr.y -= brickHeight;
            tail.emplace_back(curr);
        }
        else if (direction == D)
        {
            curr.y += brickHeight;
            tail.emplace_back(curr);
        }

        if (!checkCol(curr))
        {
            ::food.clear();
            ::spec.clear();
            lives--;
            reset();
        }

        if (tail.size() > (unsigned int)length)
        {
            tail.pop_front();
        }

        return curr;

    }

    bool changeDirection (short d)
    {
        if (direction == L || direction == R)
        {
            if (d == U || d == D) direction = d;
        }
        else if (direction == U || direction == D)
        {
            if (d == L || d == R) direction = d;
        }
        update ();
        return true;
    }

    bool isOverDraw (sf::Vector2f f)
    {
        list <SnakeB> :: iterator its = tail.begin();
        list <SnakeB> :: iterator itEnd = tail.end();

        while (its != itEnd)
        {
            if (its->pos.x == f.x && its->pos.y == f.y)
            {
                return true;
            }
            its++;
        }

        return false;
    }

    bool checkCol (sf::Vector2f f)
    {

        list <SnakeB> :: iterator its = tail.begin();
        list <SnakeB> :: iterator itEnd = tail.end();
        itEnd--;

        while (its != itEnd)
        {
            if (its->pos.x == f.x && its->pos.y == f.y)
            {
                PlaySound(TEXT("files/sounds/boom.wav"), NULL, SND_ASYNC);
                return false;
            }
            its++;
        }

        list <WallB> :: iterator itw = wall.begin();
        while (itw != wall.end())
        {
            if (itw->pos.x == f.x && itw->pos.y == f.y)
            {
                PlaySound(TEXT("files/sounds/boom.wav"), NULL, SND_ASYNC);
                return false;
            }
            itw++;
        }

        list <FoodB> :: iterator itf = food.begin();
        while (itf != food.end())
        {
            if (itf->pos.x == f.x && itf->pos.y == f.y)
            {
                PlaySound(TEXT("files/sounds/chrum.wav"), NULL, SND_ASYNC);
                changeLength(rand() % 5 + 1);
                foodEaten++;
                if (snakesSpeed > 0.01) snakesSpeed -= snakesSpeed * 0.003;
                if (foodEaten % 10 == 0)
                {
                    foodEaten = 0;
                    pointsMulti += 2;
                }
                points += itf->points * pointsMulti;
                food.erase(itf);
                break;

            }
            itf++;
        }

        list <SpecB> :: iterator itsp = spec.begin();
        while (itsp != spec.end())
        {
            if (itsp->pos.x == f.x && itsp->pos.y == f.y)
            {
                PlaySound(TEXT("files/sounds/woowoo.wav"), NULL, SND_ASYNC);
                if (itsp->type == 0)
                {
                    if (snakesSpeed <= snakesSpeedBegin) snakesSpeed += 0.01;
                }
                else if (itsp->type == 1)
                {
                    if (length > 10) changeLength(-10);
                }
                else if (itsp->type == 2)
                {
                    lives++;
                }

                spec.erase(itsp);
                break;

            }
            itsp++;
        }


        return true;
    }

    void changeLength (int r)
    {
        if (r < 0)
        {
            int w = -r;
            while (w>0)
            {
                tail.pop_front();
                w--;
            }
        }
        length += r;
    }
};

vector <Snake> snakes;

class Spec
{
public:
    sf::Vector2f pos;
    Spec()
    {
        texmgr.loadTexture("clock", imgCatalog + "clock.png");
        texmgr.loadTexture("scissors", imgCatalog + "scissors.png");
        texmgr.loadTexture("extraLive", imgCatalog + "extraLive.png");
    }

    void add (int t)
    {
        if (spec.size() > 0) spec.pop_front();
        bool overdraw = true;
        while(overdraw)
        {
            pos.x = (int)floor(((rand() % windowWidth*2)) / (brickWidth * 2)) * brickWidth;
            pos.y = (int)floor(((rand() % windowHeight*2)) / (brickHeight * 2)) * brickHeight;

            for(auto& s : snakes)
                {
                    if (!s.isOverDraw(sf::Vector2f(pos.x, pos.y)))
                    {
                        overdraw = false;
                    }
                }

        }
        if (pos.x == 0) pos.x += brickWidth;
        if (pos.y == 0) pos.y += brickHeight;
        if (pos.x >= windowWidth - brickWidth) pos.x -= brickWidth;
        if (pos.y >= windowHeight - brickHeight) pos.y -= brickHeight;

        spec.emplace_back(pos, t);
    }

    void remove ()
    {
        spec.pop_front();
    }

    void update ()
    {
        for(auto& s : spec) window.draw(s.spec_sprite);
    }
};

class Food
{
public:
    sf::Vector2f pos;
    Food()
    {
        texmgr.loadTexture("apple", imgCatalog + "apple.png");
        texmgr.loadTexture("banana", imgCatalog + "banana.png");
        texmgr.loadTexture("strawberry", imgCatalog + "strawberry.png");
        texmgr.loadTexture("pear", imgCatalog + "pear.png");
        texmgr.loadTexture("watermelon", imgCatalog + "watermelon.png");
        texmgr.loadTexture("grape", imgCatalog + "grape.png");
        texmgr.loadTexture("cherry", imgCatalog + "cherry.png");
    }

    void add (int t)
    {
        if (food.size() > 2) food.pop_front();
        bool overdraw = true;
        while(overdraw)
        {
            pos.x = (int)floor(((rand() % windowWidth*2)) / (brickWidth * 2)) * brickWidth;
            pos.y = (int)floor(((rand() % windowHeight*2)) / (brickHeight * 2)) * brickHeight;

            for(auto& s : snakes)
                {
                    if (!s.isOverDraw(sf::Vector2f(pos.x, pos.y)))
                    {
                        overdraw = false;
                    }
                }

        }

        if (pos.x == 0) pos.x += brickWidth;
        if (pos.y == 0) pos.y += brickHeight;
        if (pos.x >= windowWidth - brickWidth) pos.x -= brickWidth;
        if (pos.y >= windowHeight - brickHeight) pos.y -= brickHeight;
        food.emplace_back(pos, t);
    }

    void remove ()
    {
        food.pop_front();
    }

    void update ()
    {
        for(auto& s : food) window.draw(s.food_sprite);
    }
};

void playIntroMusic()
{
    PlaySound(TEXT("files/sounds/start.wav"), NULL, SND_ASYNC | SND_LOOP);
}

void stopIntroMusic()
{
    PlaySound(NULL, 0, 0);
}

// ############ MAIN ############ MAIN ############ MAIN ############ MAIN ############ MAIN ############ MAIN ############ MAIN
int main()
{
    bool outOfWindow = false;

    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);

    srand( time( NULL ) );

    music = true;
    playIntroMusic();

    int gameStage = 0;

    sf::Font font;

    font.loadFromFile("files/verdana.ttf");

    sf::Text gameInfo;
    gameInfo.setFont(font);
    gameInfo.setCharacterSize(22);
    gameInfo.setColor(sf::Color::Red);
    gameInfo.setStyle(sf::Text::Bold);

    sf::Text livesCounter;
    livesCounter.setFont(font);
    livesCounter.setCharacterSize(22);
    livesCounter.setColor(sf::Color::Yellow);
    livesCounter.setStyle(sf::Text::Bold);
    livesCounter.setPosition(62, windowHeight - brickHeight-30);

    sf::Text pointsCounter;
    pointsCounter.setFont(font);
    pointsCounter.setCharacterSize(22);
    pointsCounter.setColor(sf::Color::Yellow);
    pointsCounter.setStyle(sf::Text::Bold);
    pointsCounter.setPosition(152, windowHeight - brickHeight-30);

    sf::Texture begin_texture;
    begin_texture.loadFromFile(imgCatalog + "snake_welcome_begin.png");
    sf::Sprite logo_sprite;
    logo_sprite.setTexture(begin_texture);
    logo_sprite.setPosition( (windowWidth - 600) / 2, (((windowHeight - 300) / 3) * 2) / 2 );

    sf::Texture pause_texture;
    pause_texture.loadFromFile(imgCatalog + "snake_pause.png");
    sf::Sprite pause_sprite;
    pause_sprite.setTexture(pause_texture);
    pause_sprite.setPosition( (windowWidth - 600) / 2, (((windowHeight - 300) / 3) * 2) / 2 );

    sf::Texture heart_yellow_texture;
    heart_yellow_texture.loadFromFile(imgCatalog + "heart_yellow.png");
    sf::Sprite heart_yellow_sprite;
    heart_yellow_sprite.setTexture(heart_yellow_texture);
    heart_yellow_sprite.setPosition(30, windowHeight - brickHeight - brickHeight - 10);

    sf::Texture dollar_yellow_texture;
    dollar_yellow_texture.loadFromFile(imgCatalog + "dollar_yellow.png");
    sf::Sprite dollar_yellow_sprite;
    dollar_yellow_sprite.setTexture(dollar_yellow_texture);
    dollar_yellow_sprite.setPosition(120, windowHeight - brickHeight - brickHeight - 12);

    window.setFramerateLimit(60);

    llu timerToFood = 50;
    llu timerFood = timerToFood;
    int foodTypesCount = 7;

    llu timerToSpec = 500;
    llu timerSpec = timerToSpec;
    int specTypesCount = 3;

    Wall wall;
    snakes.emplace_back(Snake());
    Food food;
    Spec spec;

    sf::Clock clock;
    sf::Clock clockKey;
    sf::Time elapsed;
    sf::Time elapsedKey;
    sf::Event event;

    llu time;

    time = 0;

    window.clear();

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                window.close();
            }

            if (event.type == sf::Event::LostFocus)
                {
                    outOfWindow = true;
                    gameStage = 5;
                }
            else if (event.type == sf::Event::GainedFocus)
                {
                    outOfWindow = false;
                }

        }

        if (gameStage == 0)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                if (music)
                {
                    music = false;
                    stopIntroMusic();

                }
            }


            window.clear();
            window.draw(logo_sprite);

            gameInfo.setPosition(62, windowHeight - brickHeight-30);
            gameInfo.setString(to_string("press ENTER to start game"));
            window.draw(gameInfo);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Return))
            {
                PlaySound(NULL, 0, 0);
                gameStage = 4;
            }
        }
        else if (gameStage == 4)
        {

            window.clear();
            livesCounter.setString(to_string(snakes[0].lives));
            pointsCounter.setString(to_string(snakes[0].points));
            window.draw(livesCounter);
            window.draw(pointsCounter);
            window.draw(heart_yellow_sprite);
            window.draw(dollar_yellow_sprite);

            wall.update();

            for(auto& s : snakes) s.update();

            food.update();
            spec.update();

            elapsed = clock.getElapsedTime();

            if (elapsed.asSeconds() >= snakesSpeed)
            {
                for(auto& s : snakes)
                {
                    if (s.lives == 0) gameStage = 9;
                    s.move();
                }
                clock.restart();
                time++;
                timerFood--;
                timerSpec--;
            }

            if (timerFood <=0)
            {
                food.add(rand() % foodTypesCount);
                food.add(rand() % foodTypesCount);
                timerFood = timerToFood;
            }

            if (timerSpec <=0)
            {
                spec.add(rand() % specTypesCount);
                timerSpec = timerToSpec;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                snakes[0].changeDirection(U);
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                snakes[0].changeDirection(D);
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                snakes[0].changeDirection(L);
            }
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                snakes[0].changeDirection(R);
            }


            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                gameStage = 5;
            }
        }
        else if (gameStage == 5)
        {
            window.draw(pause_sprite);
            if (outOfWindow == false)
            {
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Return))
                {
                    gameStage = 4;
                }
            }
        }
        else if (gameStage == 9)
        {
            window.clear();
            window.draw(logo_sprite);
            ::food.clear();
            gameInfo.setPosition(90, windowHeight - brickHeight-180);
            gameInfo.setString(to_string("GAME OVER, press ENTER to play again"));
            window.draw(gameInfo);

            window.draw(dollar_yellow_sprite);

            pointsCounter.setString(to_string(snakes[0].points));
            window.draw(pointsCounter);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Return))
            {
                for(auto& s : snakes)
                {
                    s.lives = startLives;
                    s.pointsMulti = 1;
                    s.foodEaten = 0;
                    s.points = 0;
                    s.reset();
                }
                snakesSpeed = snakesSpeedBegin;
                gameStage = 4;
            }

        }


        window.display();
    }
    return EXIT_SUCCESS;
}
