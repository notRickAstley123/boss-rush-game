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
bool attackcheck = false;
float playerHealth=100;
float playerMaxHealth=100;
float bossHealth = 1000;
float bossMaxHealth=1000;
using namespace std;
float cleavecooldown = 0;
float cleaveduration = 0;
float cleavehitcooldown = 5;
float cleavehitcooldowntickdown = 0;
bool cleavin = false;
float playerArrowSpeed=8.67;
float arrowDamage = 5;
const int playerarrowcooldown = 10;
float speed = 5.f;
float bulletSpeed = 6.f;
int shootInterval = 30;
const float pi = 3.14159265f;
bool cleavehit = false;
int tickBossPhases = 0;

sf::Vector2f getCenter(const sf::Sprite& s)
{
    sf::FloatRect b = s.getGlobalBounds();
    return {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f};
}
struct playerProjectileStruct
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f direction;
};
// Should change this later to a hitbox check
bool PlayerMeleeCheck(const sf::Sprite& cleave, const sf::Sprite& boss)
{
    const sf::FloatRect cleaveBounds = cleave.getLocalBounds();
    const sf::Vector2f cleaveEnd = cleaveBounds.position + cleaveBounds.size;
    const sf::Transform transform = cleave.getTransform();
    const std::vector<sf::Vector2f> cleaveCorners = {
        transform.transformPoint(cleaveBounds.position),
        transform.transformPoint({cleaveEnd.x, cleaveBounds.position.y}),
        transform.transformPoint(cleaveEnd),
        transform.transformPoint({cleaveBounds.position.x, cleaveEnd.y})
    };

    const sf::FloatRect bossBounds = boss.getGlobalBounds();
    const sf::Vector2f bossEnd = bossBounds.position + bossBounds.size;
    const std::vector<sf::Vector2f> bossCorners = {
        bossBounds.position,
        {bossEnd.x, bossBounds.position.y},
        bossEnd,
        {bossBounds.position.x, bossEnd.y}
    };

    // Check overlap on each rectangle's edge normals (separating-axis test).
    for (const auto* polygon : {&cleaveCorners, &bossCorners})
    {
        for (std::size_t i = 0; i < polygon->size(); ++i)
        {
            const sf::Vector2f edge = (*polygon)[(i + 1) % polygon->size()] - (*polygon)[i];
            const sf::Vector2f axis = {-edge.y, edge.x};
            float cleaveMin = cleaveCorners[0].x * axis.x + cleaveCorners[0].y * axis.y;
            float cleaveMax = cleaveMin;
            float bossMin = bossCorners[0].x * axis.x + bossCorners[0].y * axis.y;
            float bossMax = bossMin;

            for (const sf::Vector2f& point : cleaveCorners)
            {
                const float projection = point.x * axis.x + point.y * axis.y;
                cleaveMin = std::min(cleaveMin, projection);
                cleaveMax = std::max(cleaveMax, projection);
            }
            for (const sf::Vector2f& point : bossCorners)
            {
                const float projection = point.x * axis.x + point.y * axis.y;
                bossMin = std::min(bossMin, projection);
                bossMax = std::max(bossMax, projection);
            }

            if (cleaveMax < bossMin || bossMax < cleaveMin)
                return false;
        }
    }
    return true;
}
void firePlayerArrow(std::vector<playerProjectileStruct>& arrows, sf::Vector2f from, sf::Vector2f target)
{
    sf::Vector2f direction = target - from;
    float length = std::sqrt(direction.x * direction.x+direction.y*direction.y);
    if (length == 0.f)
        return;
    arrows.push_back({from, (direction/length) * playerArrowSpeed});
}
void 
Boss(float amount)
{
    bossHealth -= amount;

}
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
struct enemyBullet
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
    sf::Texture playerArrowTexture;
    if (!playerArrowTexture.loadFromFile("arrow.png"))
    {
        std::cout << "file load fail\n";
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
    sf::Texture kaboomTexture;
    if (!kaboomTexture.loadFromFile("kaboom.jpg"))
    {
        return 1;
    }
    sf::Sprite kaboom(kaboomTexture);
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
    sf::Sprite arrow1(playerArrowTexture);
    arrow1.setOrigin({static_cast<float>(playerArrowTexture.getSize().x),
                      static_cast<float>(playerArrowTexture.getSize().y)});
    sf::Sprite BossHealthbarFront(PlayerHealthbarFrontTexture);
    sf::Sprite BossHealthbarBack(PlayerHealthbarBackTexture);
    sf::Sprite PlayerHealthbarBack(PlayerHealthbarBackTexture);
    sf::Sprite cleave1(cleave_texture1);
    {
        sf::Vector2f size = cleave1.getLocalBounds().size;
        cleave1.setOrigin({0.f, size.y / 2.f});
    }
    
    float iFrames=5;
    float iFrameTickdown=0;
    sf::Sprite boss1(aurafulbosstexture);
    boss1.setPosition({150.f, 150.f});

    sf::Sprite player(playertexture);
    player.setPosition({200.f, 200.f});

    sf::Sprite projectile1(bullettexture);
    {
        sf::Vector2f size = projectile1.getLocalBounds().size;
        projectile1.setOrigin({size.x / 2.f, size.y / 2.f});
    }

    PlayerHealthbarBack.setPosition({50, 50});
    PlayerHealthbarFront.setPosition({50,50});
    BossHealthbarFront.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setPosition({500, 20});
    BossHealthbarFront.setPosition({501, 21});
    std::vector<enemyBullet> bullets;
    int shootTimer = 0;
    std::vector<playerProjectileStruct> playerArrows;
    int playerarrowshootcooldown = 0;
    kaboom.setPosition({0,0});
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
        if (playerarrowshootcooldown > 0)
            playerarrowshootcooldown--;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G) && playerarrowshootcooldown == 0)
        {
            firePlayerArrow(playerArrows, playerCenter, mousePos);
            playerarrowshootcooldown = playerarrowcooldown;
        }

        std::vector<playerProjectileStruct> keptProjectiles;
        for (playerProjectileStruct a : playerArrows)
        {
            a.position += a.velocity;
            arrow1.setPosition(a.position);
            arrow1.setRotation(sf::radians(std::atan2(a.velocity.y, a.velocity.x)));

            if (arrow1.getGlobalBounds().findIntersection(boss1.getGlobalBounds()))
            {
                bossHealth -= arrowDamage;
                arrow1.setPosition({-50.f, -100.f});
            }
        

        bool offScreen = a.position.x < -50.f || a.position.x > 850.f ||
                        a.position.y < -50.f || a.position.y > 650.f;
        if (!offScreen)
            keptProjectiles.push_back(a);

        playerArrows = keptProjectiles;
        }
        float cleaveAngle = std::atan2(cleaveAim.y, cleaveAim.x);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T) && cleavecooldown == 0)
        {
            cleaveduration = 15;
            cleavecooldown = 75;
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
            cleave1.setRotation(sf::radians(std::atan2(cleaveAim.y, cleaveAim.x)));
            if (PlayerMeleeCheck(cleave1, boss1) && cleavehitcooldown ==0)
                {
                    bossHealth=bossHealth-75;
                    cleavehitcooldown = 5;
                }
        }
        
        if (cleavehitcooldown > 0)
        {
            cleavehitcooldown = cleavehitcooldown - 1;
        }
        PlayerHealthbarFront.setScale({1.f,(playerHealth/playerMaxHealth)});
        BossHealthbarFront.setScale({1.f, ((bossHealth/bossMaxHealth)*3)});
        BossHealthbarBack.setScale({1.f, 3.f});
        if (20>tickBossPhases && tickBossPhases>10)
        {
            shootInterval=13;
        }
        if (40>tickBossPhases && tickBossPhases>20)
        {
            shootInterval=10;
        }
        if (60>tickBossPhases && tickBossPhases>40)
        {
            shootInterval=7;
        }
        if (65>tickBossPhases && tickBossPhases>30)
        {
            shootInterval=5;
        }
        if (90>tickBossPhases && tickBossPhases>65)
        { 
            shootInterval=2;
            
        }
        if (tickBossPhases>90)
        {
            shootInterval=15;
            tickBossPhases=0;
            bulletSpeed = 7;
        }
        shootTimer++;
        if (shootTimer >= shootInterval)
        {
            shootTimer = 0;
            float length = std::sqrt(aim.x * aim.x + aim.y * aim.y);
            if (length > 0.f)
                bullets.push_back({bossCenter, (aim / length) * bulletSpeed});
        }

        std::vector<enemyBullet> remaining;
        for (enemyBullet b : bullets)
        {
            b.position += b.velocity;
            projectile1.setPosition(b.position);

            bool offScreen = b.position.x < -50.f || b.position.x > 850.f ||
                             b.position.y < -50.f || b.position.y > 650.f;

            if (player.getGlobalBounds().findIntersection(projectile1.getGlobalBounds()))
            {
                if (iFrames==0)
                {
                    playerHealth=playerHealth-10;
                    iFrames=5;
                }
            }

            if (!offScreen)
            {
                remaining.push_back(b);
            }
        }
        if (iFrames>0)
        {
            iFrames=iFrames-1;
        }
        bullets = remaining;
        // Drawing stuff, singular items only 
        window.clear(sf::Color::Black);
        window.draw(player);
        window.draw(boss1);
        window.draw(PlayerHealthbarBack);
        window.draw(PlayerHealthbarFront);
        window.draw(BossHealthbarBack);
        window.draw(BossHealthbarFront);



        for (const enemyBullet& b : bullets)
        {
            projectile1.setPosition(b.position);
            window.draw(projectile1);
        }
        for (const playerProjectileStruct& a : playerArrows)
        {
            arrow1.setPosition(a.position);
            arrow1.setRotation(sf::radians(std::atan2(a.velocity.y, a.velocity.x)));
            window.draw(arrow1);
        }
        if (cleavin)
            window.draw(cleave1);
        if (bossHealth < 1)
        {
            window.draw(kaboom);
        }
        window.display();
        if (playerHealth < 1)
        {
            window.draw(kaboom);
        }
        tickBossPhases=tickBossPhases+1;
    }

    return 0;
}
