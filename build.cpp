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
// Setup
using namespace std;
float cleavecooldown = 0;
float cleaveduration = 0;
bool cleavin = false;
float speed = 5.f;
const float bulletSpeed = 6.f;
const int shootInterval = 30;
const float pi = 3.14159265f;
bool cleavehit = false;

sf::Vector2f getCenter(const sf::Sprite& s)
{
    sf::FloatRect b = s.getGlobalBounds();
    return {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f};
}
// Should change this later to a hitbox check
bool cleaveHitsBoss(const sf::Sprite& cleave, sf::Vector2f bossCenter,
                      sf::Vector2f aim, const sf::Sprite& player)
{
    float aimLength = std::sqrt(aim.x * aim.x + aim.y * aim.y);
    if (aimLength == 0.f)
        return false;

    sf::Vector2f u = aim / aimLength;   
    sf::Vector2f n = {-u.y, u.x};       
    // The cleave hitbox is rotated
    sf::Vector2f cleaveSize = cleave.getLocalBounds().size;
    float length = cleaveSize.x;        
    float halfWidth = cleaveSize.y / 2.f;


    sf::FloatRect pb = player.getGlobalBounds();
    // Treats the player as a circle 
    float playerRadius = std::max(pb.size.x, pb.size.y) / 2.f;

    sf::Vector2f d = getCenter(player) - bossCenter;

    float along = d.x * u.x + d.y * u.y;
    float side = d.x * n.x + d.y * n.y;

    return along >= -playerRadius && along <= length + playerRadius &&
           std::abs(side) <= halfWidth + playerRadius;

}
float bossHealth = 100;
void damageBoss(float amount)
{
    bossHealth -= amount;

}
// Not used yet, should be used later once I set up a heath bar system
struct HealthBar
{
    sf::Vector2f position;
    sf::Vector2f location;
    sf::Vector2f direction;
};
struct HealthBarBack
{
    sf::Vector2f position;
    sf::Vector2f location;
    sf::Vector2f direction;
};
// Used for the projectiles
struct Bullet
{
    sf::Vector2f position;
    sf::Vector2f velocity;
};

int main()
{
    // 800 by 600 pixels
    sf::RenderWindow window(sf::VideoMode({800, 600}), "boss rush game");
    // 30 frames per second
    window.setFramerateLimit(30);
    
    // Loading textures
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
    sf::Texture PlayerHealthbarFrontTexture;
    if (!PlayerHealthbarFrontTexture.loadFromFile("healthbar.png"))
    {
        std::cout << "file load fail4\n";
        std::cin.get();
        return 1;
    }
    sf::Sprite PlayerHealthbarFront(PlayerHealthbarFrontTexture);
    sf::Texture PlayerHealthbarBackTexture;
    if (!PlayerHealthbarBackTexture.loadFromFile("healthbarback.png"))
    {
        std::cout << "file load fail4\n";
        std::cin.get();
        return 1;
    }
    sf::Sprite PlayerHealthbarBack(PlayerHealthbarBackTexture);
    sf::Sprite cleave1(cleave_texture1);
    {
        sf::Vector2f size = cleave1.getLocalBounds().size;
        cleave1.setOrigin({0.f, size.y / 2.f});
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
    PlayerHealthbarBack.setOrigin({2.f, 2.f});
    PlayerHealthbarFront.setOrigin({2.f, 2.f});
    std::vector<Bullet> bullets;
    int shootTimer = 0;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        // Player movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            player.move({0.f, -speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            player.move({0.f, speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.move({-speed, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.move({speed, 0.f});

        sf::Vector2f bossCenter = getCenter(boss1);
        sf::Vector2f playerCenter = getCenter(player);
        // Unused
        sf::Vector2f aim = playerCenter - bossCenter;
        // Attack is aimed towards the cursor
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        sf::Vector2f cleaveAim = mousePos - playerCenter;
        float cleaveAngle = std::atan2(cleaveAim.y, cleaveAim.x);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T) && cleavecooldown == 0)
        {
            cleaveduration = 12;
            cleavecooldown = 60;
            cleavehit = false;
        }

        // Count down the swing and the cooldown
        cleavin = cleaveduration > 0;
        if (cleaveduration > 0)
            cleaveduration--;
        if (cleavecooldown > 0)
            cleavecooldown--;


        if (cleavin)
        {
            cleave1.setPosition(playerCenter);
            cleave1.setRotation(sf::radians(cleaveAngle));
        }
        shootTimer++;
        if (shootTimer >= shootInterval)
        {
            shootTimer = 0;
            float length = std::sqrt(aim.x * aim.x + aim.y * aim.y);
            if (length > 0.f)
                bullets.push_back({bossCenter, (aim / length) * bulletSpeed});
        }

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
        // Drawing stuff
        window.clear(sf::Color::Black);
        window.draw(player);
        window.draw(boss1);
        window.draw(PlayerHealthbarBack);
        window.draw(PlayerHealthbarFront);

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
