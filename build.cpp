#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;
float cleavecooldown = 60;
float cleaveduration = 0;
bool cleavin = false;
float speed = 5.f;
const float bulletSpeed = 6.f;
const int shootInterval = 30;
const float pi = 3.14159265f;

void projectilehitcheck(const sf::Sprite& a, const sf::Sprite& b)
{
    if (a.getGlobalBounds().findIntersection(b.getGlobalBounds()))
    {
        std::cout << "hit!\n";
    }
}

sf::Vector2f getCenter(const sf::Sprite& s)
{
    sf::FloatRect b = s.getGlobalBounds();
    return {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f};
}

struct Bullet
{
    sf::Vector2f position;
    sf::Vector2f velocity;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "My Boss Rush Game");
    window.setFramerateLimit(30);

    sf::Texture playertexture;
    if (!playertexture.loadFromFile("pixels.png"))
    {
        std::cout << "file load fail1\n";
        std::cin.get();
        return 1;
    }

    sf::Texture bullettexture;
    if (!bullettexture.loadFromFile("bullet.png"))
    {
        std::cout << "file load fail2\n";
        std::cin.get();
        return 1;
    }

    sf::Texture cleave_texture1;
    if (!cleave_texture1.loadFromFile("cleave.png"))
    {
        std::cout << "file load fail3\n";
        std::cin.get();
        return 1;
    }

    sf::Texture aurafulbosstexture;
    if (!aurafulbosstexture.loadFromFile("auraful_tm_opera_o.png"))
    {
        std::cout << "file load fail4\n";
        std::cin.get();
        return 1;
    }

    sf::Sprite cleave1(cleave_texture1);
        {
        sf::Vector2f size = cleave1.getLocalBounds().size;
        cleave1.setOrigin({0.f, size.y / 2.f}); // x = middle, y = top edge
        }


    sf::Sprite boss1(aurafulbosstexture);
    boss1.setPosition({150.f, 150.f});

    sf::Sprite player(playertexture);
    player.setPosition({200.f, 200.f});

    sf::Sprite projectile1(bullettexture);
    {
        sf::Vector2f size = projectile1.getLocalBounds().size;
        projectile1.setOrigin({size.x / 2.f, size.y / 2.f});
    }

    std::vector<Bullet> bullets;
    int shootTimer = 0;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            player.move({0.f, -speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            player.move({0.f, speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.move({-speed, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.move({speed, 0.f});

        // Aim from boss center to player center
        sf::Vector2f bossCenter = getCenter(boss1);
        sf::Vector2f playerCenter = getCenter(player);
        sf::Vector2f aim = playerCenter - bossCenter;
        float angle1 = std::atan2(aim.y, aim.x);

        // Cleave timer
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T))
        {
            cleaveduration = 12;
        }
        if (cleaveduration > 0)
        {
            cleaveduration -= 1;
            cleavin = true;
        }
        else
        {
            cleavin = false;
        }

        if (cleavin)
        {
            cleave1.setPosition(playerCenter+sf::Vector2f{0.f,20.f});
            cleave1.setRotation(sf::radians(angle1 + pi));
        }

        // Boss shoots bullets
        shootTimer++;
        if (shootTimer >= shootInterval)
        {
            shootTimer = 0;
            float length = std::sqrt(aim.x * aim.x + aim.y * aim.y);
            if (length > 0.f)
            {
                bullets.push_back({bossCenter, (aim / length) * bulletSpeed});
            }
        }

        // Move bullets, remove off-screen ones, check hits
        std::vector<Bullet> remaining;
        for (Bullet b : bullets)
        {
            b.position += b.velocity;
            projectile1.setPosition(b.position);

            bool offScreen = b.position.x < -50.f || b.position.x > 850.f ||
                             b.position.y < -50.f || b.position.y > 650.f;

            if (player.getGlobalBounds().findIntersection(projectile1.getGlobalBounds()))
            {
                std::cout << "oof\n";
                continue;
            }
            if (!offScreen)
                remaining.push_back(b);
        }
        bullets = remaining;

        // Cleave collision
        if (cleavin)
            projectilehitcheck(player, boss1);

        // Drawing
        window.clear(sf::Color::Black);
        window.draw(player);
        window.draw(boss1);

        for (const Bullet& b : bullets)
        {
            projectile1.setPosition(b.position);
            window.draw(projectile1);
        }

        if (cleavin)
            window.draw(cleave1);

        window.display();
    }

    return 0;
}
