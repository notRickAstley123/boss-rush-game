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
bool arrowCharging = false;
bool attackcheck = false;
float playerHealth=100;
float playerMaxHealth=100;
float bossHealth = 500;
float bossMaxHealth=500;
using namespace std;
float cleavecooldown = 0;
float cleaveduration = 0;
float cleavehitcooldown = 5;
float cleavehitcooldowntickdown = 0;
bool cleavin = false;
float playerArrowSpeed=1;
float arrowDamage = 1;
int playerarrowcooldown = 10;
float speed = 5.f;
float bulletSpeed = 6.f;
int shootInterval = 30;
const float pi = 3.14159265f;
bool cleavehit = false;
int tickBossPhases = 0;
int dashCDtickdown = 0;
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
    float damage;
    float arrowSpeed;
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

void firePlayerArrow(std::vector<playerProjectileStruct>& arrows, sf::Vector2f from, sf::Vector2f target, float damage, float arrowSpeed)
{
    sf::Vector2f direction = target - from;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length == 0.f)
        return;
    arrows.push_back({from, (direction / length) * arrowSpeed, {}, damage, arrowSpeed});
}
void 
Boss(float amount)
{
    bossHealth -= amount;

}
void bossFight1()
{

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
struct button
{
    sf::Vector2f position;
    sf::Vector2f location;
    sf::Vector2f size;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({1000, 700}), "boss rush game");
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
    sf::Texture WhiteButtonBackgroundTexture;
    // edit later
    if (WhiteButtonBackgroundTexture.loadFromFile("nothing yet"))
    {
        std::cout << "file load failed\n";
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
    sf::Texture greenKaboomTexture;
    if (!greenKaboomTexture.loadFromFile("greenkaboom.jpg"))
    {
        std::cout << "file load fail\n";
        std::cin.get();
        return 1;
    }     
    sf::Sprite greenKaboom(greenKaboomTexture);
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
    PlayerHealthbarFront.setPosition({51,51});
    BossHealthbarFront.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setPosition({500, 20});
    BossHealthbarFront.setPosition({502, 22});
    std::vector<enemyBullet> bullets;
    int shootTimer = 0;
    std::vector<playerProjectileStruct> playerArrows;
    int playerarrowshootcooldown = 0;
    kaboom.setPosition({0,0});
    greenKaboom.setPosition({0,0});
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        // Player movement
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(-speed*15), (-speed*15)});
            speed=2;
            dashCDtickdown=15;
        }
        if((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(speed*15),(-speed*15)});
            speed=2;
            dashCDtickdown=15;

        }
        if((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(speed*15),(speed*15)});
            speed=2;
            dashCDtickdown=15;
        }
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(-speed*15), (speed*15)});
            speed=2;
            dashCDtickdown=15;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({0.f, (-speed*30)});
            speed=2;
            dashCDtickdown=15;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({0.f, (speed*30)});
            speed=2;
            dashCDtickdown=15;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(-speed*30), 0.f});
            speed=2;
            dashCDtickdown=15;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)&&(dashCDtickdown==0)&&(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)))
        {
            player.move({(speed*30), 0.f});
            speed = 2;
            dashCDtickdown=15;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            player.move({(speed), 0.f});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            player.move({(-speed), 0.f});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            player.move({0.f,(-speed)});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            player.move({0.f, (speed)});
        }
        if (dashCDtickdown > 0)
        {
            dashCDtickdown=dashCDtickdown-1;
        }
        if (dashCDtickdown == 0)
        {
            speed = 5;
        }

        sf::Vector2f bossCenter = getCenter(boss1);
        sf::Vector2f playerCenter = getCenter(player);
        // Unused
        sf::Vector2f aim = playerCenter - bossCenter;
        sf::Vector2f test = bossCenter - playerCenter;
        // idk
        // Attack is aimed towards the cursor
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        sf::Vector2f cleaveAim = mousePos - playerCenter;
        if (playerarrowshootcooldown > 0)
            playerarrowshootcooldown--;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G) && playerarrowshootcooldown == 0)
        {
            arrowCharging = true;
        }
        if (arrowCharging==true)
        {
            if (arrowDamage<300)
            {
                arrowDamage+=5.f;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G))
            {
                float arrowSpeed = std::min(4.f + arrowDamage / 20.f, 20.f);
                firePlayerArrow(playerArrows, playerCenter, mousePos, arrowDamage, arrowSpeed);

                playerarrowshootcooldown = playerarrowcooldown; 
                arrowCharging=false;
            }
        }
        std::vector<playerProjectileStruct> keptProjectiles;
        for (const playerProjectileStruct& projectile : playerArrows)
        {
            playerProjectileStruct a = projectile;
            a.position += a.velocity;
            arrow1.setPosition(a.position);
            arrow1.setRotation(sf::radians(std::atan2(a.velocity.y, a.velocity.x)));

            const bool hitBoss = arrow1.getGlobalBounds().findIntersection(boss1.getGlobalBounds()).has_value();
            if (hitBoss)
            {
                bossHealth = std::max(0.f, bossHealth - a.damage);
                arrowDamage = 10;
                
            }
            const bool offScreen = a.position.x < -50.f || a.position.x > 850.f ||
                                   a.position.y < -50.f || a.position.y > 650.f;
            if (!hitBoss && !offScreen)
                keptProjectiles.push_back(a);
        }
        playerArrows = std::move(keptProjectiles);
        float cleaveAngle = std::atan2(cleaveAim.y, cleaveAim.x);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T) && cleavecooldown == 0)
        {
            cleaveduration = 12;
            cleavecooldown = 50;
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
                    bossHealth=bossHealth-100;
                    cleavehitcooldown = 10;
                }
        }
        
        if (cleavehitcooldown > 0)
        {
            cleavehitcooldown = cleavehitcooldown - 1;
        }
        PlayerHealthbarFront.setScale({1.f,(playerHealth/playerMaxHealth)});
        BossHealthbarFront.setScale({1.f, ((bossHealth/bossMaxHealth)*3)});
        BossHealthbarBack.setScale({1.f, 3.f});
        if (tickBossPhases > 10 && tickBossPhases <= 20)
        {
            shootInterval=13;
            boss1.setPosition({200.f, 300.f});
        }
        if (tickBossPhases > 20 && tickBossPhases <= 40)
        {
            shootInterval=10;
            boss1.setPosition({300.f, 300.f});
        }
        if (tickBossPhases > 40 && tickBossPhases <= 60)
        {
            shootInterval=7;
            boss1.setPosition({400.f, 300.f});
        }
        if (tickBossPhases > 60 && tickBossPhases <= 65)
        {
            shootInterval=5;
            boss1.setPosition({500.f, 300.f});
        }
        if (tickBossPhases > 65 && tickBossPhases <= 90)
        { 
            shootInterval=2;
            boss1.setPosition({600.f, 300.f});
            
        }
        if (tickBossPhases>90)
        {
            shootInterval=15;
            tickBossPhases=0;
            bulletSpeed = 7;
            boss1.setPosition({100.f, 300.f});
        }
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
        std::vector<enemyBullet> remaining;
        for (const enemyBullet& bullet : bullets)
        {
            enemyBullet b = bullet;
            b.position += b.velocity;
            projectile1.setPosition(b.position);

            bool offScreen = b.position.x < -50.f || b.position.x > 850.f ||
                             b.position.y < -50.f || b.position.y > 650.f;

            const bool hitPlayer = player.getGlobalBounds().findIntersection(projectile1.getGlobalBounds()).has_value();
            if (hitPlayer)
            {
                if (iFrames==0)
                {
                    playerHealth = std::max(0.f, playerHealth - 7.5f);
                    iFrames=3;
                }
            }

            if (!offScreen && !hitPlayer)
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
        {
            window.draw(cleave1);
        }
        if (bossHealth < 1)
        {
            window.draw(greenKaboom);
        }
        if (playerHealth < 1)
        {
            window.draw(kaboom);
        }
        window.display();
        tickBossPhases=tickBossPhases+1;
    }
    return 0;
}



