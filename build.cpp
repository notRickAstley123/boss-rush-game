#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;


struct MeleeWeapon
{   
    string name;
    float damage;
    int cooldown;      
    int activeFrames; 
    float scale;     
};

struct RangedWeapon
{
    string name;
    float minDamage;
    float maxDamage;
    float chargePerFrame; 
    float baseSpeed;
    float maxSpeed;
    int cooldown;
};
struct UtilityGear
{
    string name;
    int effectType;
    sf::Vector2f velocity;
    sf::Vector2f size;

};
const vector<MeleeWeapon> meleeWeapons = {
    {"Cleave",      100.f, 50, 12, 1.0f},
    {"Quick Slash",  50.f, 25,  8, 0.7f},
    {"big sword", 150.f, 100, 30, 1.3f}
};

const vector<RangedWeapon> rangedWeapons = {
    {"Longbow",    10.f, 300.f, 5.f, 4.f, 20.f, 10},
    {"Quick Shot", 15.f,  15.f, 0.f, 12.f, 12.f, 5},
    {"Heavy Shot", 1.f, 600.f, 2.f, 3.f, 20.f, 20}
    
};
const vector<UtilityGear> UtilityItems = {
    {"grenade", 1}
};
// Setup
bool arrowCharging = false;
float playerHealth = 100;
float playerMaxHealth = 100;
float bossHealth = 1000;
float bossMaxHealth = 1000;
float cleavecooldown = 0;
float cleaveduration = 0;
bool cleavin = false;
bool meleeHitThisSwing = false;
float arrowDamage = 10;
float speed = 5.f;
float bulletSpeed = 6.f;
int shootInterval = 30;
int tickBossPhases = 0;
int dashCDtickdown = 0;
float boss1Speed = 0;
float bossToPlayerDistanceNum;
int bossDashAttackCooldown = 0;
float pi = 3.14159265;
enum class GameState
{
    MainMenu,
    GearSelect,
    Playing,
    Paused
};

sf::Vector2f getCenter(const sf::Sprite& s)
{
    sf::FloatRect b = s.getGlobalBounds();
    return {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f};
}
struct playerProjectileStruct
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float damage;
    
};

// Used for the boss projectiles
struct enemyBullet
{
    sf::Vector2f position;
    sf::Vector2f velocity;
};

bool loadOrFail(sf::Texture& texture, const string& path)
{
    if (texture.loadFromFile(path))
        return true;
    cout << "could not load " << path << "\n";
    cin.get();
    return false;
}
void UtilityUse(int effectType, sf::Vector2f from, sf::Vector2f target, sf::Vector2f velocity)
{
    if (effectType==1)
    {

    }
}
// Separating-axis test between the swing and the boss
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

void firePlayerArrow(vector<playerProjectileStruct>& arrows, sf::Vector2f from, sf::Vector2f target, float damage, float arrowSpeed)
{
    sf::Vector2f direction = target - from;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length == 0.f)
        return;
    arrows.push_back({from, (direction / length) * arrowSpeed, damage});
}

// ---------- Menu classes ----------
class Button
{
public:
    Button(const sf::Font& font, const string& text, sf::Vector2f position, sf::Vector2f size)
        : shape(size), label(font, sf::String(text), 28)
    {
        shape.setPosition(position);
        shape.setFillColor(sf::Color(60, 60, 60));

        const sf::FloatRect tb = label.getLocalBounds();
        label.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
        label.setPosition(position + size / 2.f);
    }

    bool contains(sf::Vector2f point) const
    {
        return shape.getGlobalBounds().contains(point);
    }

    void draw(sf::RenderTarget& target, bool highlighted) const
    {
        sf::RectangleShape s = shape;
        s.setFillColor(highlighted ? sf::Color(90, 130, 90) : sf::Color(60, 60, 60));
        target.draw(s);
        target.draw(label);
    }

private:
    sf::RectangleShape shape;
    sf::Text label;
};

class MainMenu
{
public:
    enum class Action { None, Play, Quit };

    explicit MainMenu(const sf::Font& font)
        : title(font, sf::String("game !"), 64),
          playButton(font, "Play", {400.f, 260.f}, {200.f, 60.f}),
          quitButton(font, "Quit", {400.f, 360.f}, {200.f, 60.f})
    {
        title.setPosition({300.f, 120.f});
    }
    Action onClick(sf::Vector2f mouse) const
    {
        if (playButton.contains(mouse)) return Action::Play;
        if (quitButton.contains(mouse)) return Action::Quit;
        return Action::None;

    }

    void draw(sf::RenderTarget& target, sf::Vector2f mouse) const
    {
        target.draw(title);
        playButton.draw(target, playButton.contains(mouse));
        quitButton.draw(target, quitButton.contains(mouse));
    }
private:
    sf::Text title;
    Button playButton;
    Button quitButton;
};

class GearMenu
{
public:
    enum class Action { None, Start, Back };

    explicit GearMenu(const sf::Font& font)
        : title(font, sf::String("SELECT YOUR GEAR"), 48),
          meleeLabel(font, sf::String("Melee"), 32),
          rangedLabel(font, sf::String("Ranged"), 32),
          summary(font, sf::String(""), 24),
          startButton(font, "Start", {400.f, 540.f}, {200.f, 55.f}),
          backButton(font, "Back", {400.f, 615.f}, {200.f, 55.f})
    {
        title.setPosition({250.f, 60.f});
        meleeLabel.setPosition({150.f, 170.f});
        rangedLabel.setPosition({570.f, 170.f});
        summary.setPosition({150.f, 470.f});

        for (std::size_t i = 0; i < meleeWeapons.size(); ++i)
        {
            meleeButtons.emplace_back(font, meleeWeapons[i].name,
                                      sf::Vector2f{150.f, 230.f + static_cast<float>(i) * 70.f},
                                      sf::Vector2f{280.f, 55.f});
        }
        for (std::size_t i = 0; i < rangedWeapons.size(); ++i)
        {
            rangedButtons.emplace_back(font, rangedWeapons[i].name,
                                       sf::Vector2f{570.f, 230.f + static_cast<float>(i) * 70.f},
                                       sf::Vector2f{280.f, 55.f});
        }

        updateSummary();
    }

    Action onClick(sf::Vector2f mouse)
    {
        for (std::size_t i = 0; i < meleeButtons.size(); ++i)
        {
            if (meleeButtons[i].contains(mouse))
            {
                selectedMelee = i;
                updateSummary();
                return Action::None;
            }
        }
        for (std::size_t i = 0; i < rangedButtons.size(); ++i)
        {
            if (rangedButtons[i].contains(mouse))
            {
                selectedRanged = i;
                updateSummary();
                return Action::None;
            }
        }
        if (startButton.contains(mouse)) return Action::Start;
        if (backButton.contains(mouse)) return Action::Back;
        return Action::None;
    }

    void draw(sf::RenderTarget& target, sf::Vector2f mouse) const
    {
        target.draw(title);
        target.draw(meleeLabel);
        target.draw(rangedLabel);

        for (std::size_t i = 0; i < meleeButtons.size(); ++i)
            meleeButtons[i].draw(target, i == selectedMelee || meleeButtons[i].contains(mouse));
        for (std::size_t i = 0; i < rangedButtons.size(); ++i)
            rangedButtons[i].draw(target, i == selectedRanged || rangedButtons[i].contains(mouse));

        target.draw(summary);
        startButton.draw(target, startButton.contains(mouse));
        backButton.draw(target, backButton.contains(mouse));
    }

    std::size_t meleeChoice() const { return selectedMelee; }
    std::size_t rangedChoice() const { return selectedRanged; }

private:
    void updateSummary()
    {
        const string text = "Melee: " + meleeWeapons[selectedMelee].name +
                            "     Ranged: " + rangedWeapons[selectedRanged].name;
        summary.setString(sf::String(text));
    }


    sf::Text title;
    sf::Text meleeLabel;
    sf::Text rangedLabel;
    sf::Text summary;
    Button startButton;
    Button backButton;
    vector<Button> meleeButtons;
    vector<Button> rangedButtons;
    std::size_t selectedMelee = 0;
    std::size_t selectedRanged = 0;
};

// ---------- Main ----------
int main()
{
    sf::RenderWindow window(sf::VideoMode({1000, 700}), "boss rush game");
    // 30 frames per second
    window.setFramerateLimit(30);

    // Loading the font
    sf::Font font;
    if (!font.openFromFile("arial.ttf"))
    {
        cout << "no font :< \n";
        cin.get();
        return 1;
    }

    // Loading textures
    sf::Texture playertexture, bullettexture, greenKaboomTexture, playerArrowTexture, cleave_texture1, kaboomTexture, aurafulbosstexture, PlayerHealthbarFrontTexture, PlayerHealthbarBackTexture, grenadeTexture;
    if (!loadOrFail(playertexture, "pixels.png")) return 1;
    if (!loadOrFail(bullettexture, "bullet.png")) return 1;
    if (!loadOrFail(greenKaboomTexture, "greenkaboom.jpg")) return 1;
    if (!loadOrFail(playerArrowTexture, "arrow.png")) return 1;
    if (!loadOrFail(cleave_texture1, "cleave.png")) return 1;
    if (!loadOrFail(kaboomTexture, "kaboom.jpg")) return 1;
    if (!loadOrFail(aurafulbosstexture, "auraful_tm_opera_o.png")) return 1;
    if (!loadOrFail(PlayerHealthbarFrontTexture, "healthbar.png")) return 1;
    if (!loadOrFail(PlayerHealthbarBackTexture, "healthbarback.png")) return 1;
    if (!loadOrFail(grenadeTexture, "grenade.png")) return 1;
    sf::Sprite grenade(grenadeTexture);
    sf::Sprite greenKaboom(greenKaboomTexture);
    sf::Sprite kaboom(kaboomTexture);
    kaboom.setPosition({0, 0});
    greenKaboom.setPosition({0, 0});

    sf::Sprite arrow1(playerArrowTexture);
    arrow1.setOrigin({static_cast<float>(playerArrowTexture.getSize().x) / 2.f,
                      static_cast<float>(playerArrowTexture.getSize().y) / 2.f});

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


    sf::Sprite PlayerHealthbarFront(PlayerHealthbarFrontTexture);
    sf::Sprite PlayerHealthbarBack(PlayerHealthbarBackTexture);
    sf::Sprite BossHealthbarFront(PlayerHealthbarFrontTexture);
    sf::Sprite BossHealthbarBack(PlayerHealthbarBackTexture);

    PlayerHealthbarBack.setPosition({50, 50});
    PlayerHealthbarFront.setPosition({51, 51});
    BossHealthbarFront.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setRotation(sf::degrees(90.f));
    BossHealthbarBack.setPosition({500, 20});
    BossHealthbarFront.setPosition({502, 22});

    // Text
    sf::Text pauseText(font, sf::String("PAUSED - press Esc to resume"), 36);
    pauseText.setPosition({230.f, 300.f});

    sf::Text endText(font, sf::String("Press Enter to return to the menu"), 28);
    endText.setPosition({260.f, 620.f});

    // Menus and state
    MainMenu mainMenu(font);
    GearMenu gearMenu(font);
    GameState state = GameState::MainMenu;

    // Weapons copied in when Start is pressed
    MeleeWeapon melee = meleeWeapons[0];
    RangedWeapon ranged = rangedWeapons[0];

    // Gameplay state
    vector<enemyBullet> bullets;
    vector<playerProjectileStruct> playerArrows;
    int shootTimer = 0;
    int playerarrowshootcooldown = 0;
    int iFrames = 0;
    bool gWasHeld = false;
    auto resetGame = [&]()
    {
    playerHealth = playerMaxHealth;
    bossHealth = bossMaxHealth;
    tickBossPhases = 0;
    shootInterval = 30;
    shootTimer = 0;
    dashCDtickdown = 0;
    cleavecooldown = 0;
    cleaveduration = 0;
    cleavin = false;
    meleeHitThisSwing = false;
    arrowCharging = false;
    arrowDamage = ranged.minDamage;
    playerarrowshootcooldown = 0;
    iFrames = 0;
    gWasHeld = false;
    bulletSpeed = 6.f;
    bullets.clear();
    playerArrows.clear();
    player.setPosition({200.f, 200.f});
    boss1.setPosition({150.f, 150.f});
    cleave1.setScale({melee.scale, melee.scale});
    };
    kaboom.setPosition({0,0});
    greenKaboom.setPosition({0,0});
    while (window.isOpen())
    {
        const bool gameOver = playerHealth < 1.f || bossHealth < 1.f;

        while (const optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape)
                {
                    if (state == GameState::Playing && !gameOver)
                        state = GameState::Paused;
                    else if (state == GameState::Paused)
                        state = GameState::Playing;
                }
                if (key->code == sf::Keyboard::Key::Enter && state == GameState::Playing && gameOver)
                    state = GameState::MainMenu;
            }

            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (click->button == sf::Mouse::Button::Left)
                {
                    const sf::Vector2f clickPos = static_cast<sf::Vector2f>(click->position);

                    if (state == GameState::MainMenu)
                    {
                        const MainMenu::Action action = mainMenu.onClick(clickPos);
                        if (action == MainMenu::Action::Play)
                            state = GameState::GearSelect;
                        else if (action == MainMenu::Action::Quit)
                            window.close();
                    }
                    else if (state == GameState::GearSelect)
                    {
                        const GearMenu::Action action = gearMenu.onClick(clickPos);
                        if (action == GearMenu::Action::Start)
                        {
                            melee = meleeWeapons[gearMenu.meleeChoice()];
                            ranged = rangedWeapons[gearMenu.rangedChoice()];
                            resetGame();
                            state = GameState::Playing;
                        }
                        else if (action == GearMenu::Action::Back)
                        {
                            state = GameState::MainMenu;
                        }
                    }
                }
            }
        }

        const sf::Vector2f mouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        switch (state)
        {
        case GameState::MainMenu:
            window.clear(sf::Color(20, 20, 30));
            mainMenu.draw(window, mouse);
            window.display();
            break;

        case GameState::GearSelect:
            window.clear(sf::Color(20, 20, 30));
            gearMenu.draw(window, mouse);
            window.display();
            break;

        case GameState::Paused:
            window.clear(sf::Color::Black);
            window.draw(pauseText);
            window.display();
            break;

        case GameState::Playing:
        {
            // ----- Game over screen -----
            if (gameOver)
            {
                window.clear(sf::Color::Black);
                if (bossHealth < 1.f)
                    window.draw(greenKaboom);
                if (playerHealth < 1.f)
                    window.draw(kaboom);
                window.draw(endText);
                window.display();
                break;
            }
            
            // ----- Boss phases (before aiming, so shots use the current position) -----
            if (tickBossPhases > 90)
            {
                tickBossPhases = 0;
                shootInterval = 15;
                bulletSpeed = 7.f;
            }
            else if (tickBossPhases > 65)
            {
                shootInterval = 2;
            }
            else if (tickBossPhases > 60)
            {
                shootInterval = 5;
            }
            else if (tickBossPhases > 40)
            {
                shootInterval = 7;
            }
            else if (tickBossPhases > 20)
            {
                shootInterval = 10;
            }
            else if (tickBossPhases > 10)
            {
                shootInterval = 13;
            }

            // ----- Dash: hold Q and a direction, 15-frame cooldown -----
            if (dashCDtickdown > 0)
                dashCDtickdown--;

            bool dashedThisFrame = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) && dashCDtickdown == 0)
            {
                sf::Vector2f dir{0.f, 0.f};
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dir.y -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dir.y += 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dir.x -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dir.x += 1.f;

                const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 0.f)
                {
                    player.move((dir / len) * (speed * 15.f));
                    dashCDtickdown = 15;
                    dashedThisFrame = true;
                }
            }

            // ----- Walking: skipped on dash frames -----
            if (!dashedThisFrame)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) player.move({0.f, -speed});
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) player.move({0.f, speed});
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) player.move({-speed, 0.f});
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) player.move({speed, 0.f});
            }

            // Keep the player inside the window
            {
                const sf::Vector2f pSize = player.getGlobalBounds().size;
                sf::Vector2f p = player.getPosition();
                p.x = std::clamp(p.x, 0.f, 1000.f - pSize.x);
                p.y = std::clamp(p.y, 0.f, 700.f - pSize.y);
                player.setPosition(p);
            }

            const sf::Vector2f bossCenter = getCenter(boss1);
            const sf::Vector2f playerCenter = getCenter(player);
            const sf::Vector2f aim = playerCenter - bossCenter;
            const sf::Vector2f cleaveAim = mouse - playerCenter;
            sf::Vector2f playerBossOffset = playerCenter-bossCenter;
            bossToPlayerDistanceNum=(sqrt((playerBossOffset.x*playerBossOffset.x)+(playerBossOffset.y*playerBossOffset.y)));
            
            if (playerarrowshootcooldown > 0)
                playerarrowshootcooldown--;
            if (bossToPlayerDistanceNum >100) 
            {
                
                boss1.move(playerBossOffset*-20.f);
                

            }
            if ( bossToPlayerDistanceNum > 400 && bossDashAttackCooldown == 0)
            {
                boss1.setRotation(sf::radians(std::atan2(aim.y, aim.x)));
                boss1.move({0.f,0.f});

            }
            const bool gHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G);
            const bool bHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::B);

            if (bHeld==true)
            {
                cout << "not workin yet";
                
            };
            auto fireArrow = [&]()
            {
                const float arrowSpeed = std::min(ranged.baseSpeed + arrowDamage / 20.f, ranged.maxSpeed);
                firePlayerArrow(playerArrows, playerCenter, mouse, arrowDamage, arrowSpeed);
                playerarrowshootcooldown = ranged.cooldown;
                arrowDamage = ranged.minDamage;
                arrowCharging = false;
            };

            if (ranged.chargePerFrame > 0.f)
            {
                if (gHeld && playerarrowshootcooldown == 0)
                {
                    arrowCharging = true;
                    arrowDamage = std::min(arrowDamage + ranged.chargePerFrame, ranged.maxDamage);
                }
                else if (!gHeld && arrowCharging)
                {
                    fireArrow();
                }
            }
            else if (gHeld && !gWasHeld && playerarrowshootcooldown == 0)
            {
                fireArrow();
                
            }
            gWasHeld = gHeld;

            // Move arrows, damage the boss on hit, keep only arrows still in play
            vector<playerProjectileStruct> keptProjectiles;
            for (playerProjectileStruct a : playerArrows)
            {
                a.position += a.velocity;
                arrow1.setPosition(a.position);
                arrow1.setRotation(sf::radians(std::atan2(a.velocity.y, a.velocity.x)));

                const bool hitBoss = arrow1.getGlobalBounds()
                                         .findIntersection(boss1.getGlobalBounds())
                                         .has_value();
                if (hitBoss)
                    bossHealth = std::max(0.f, bossHealth - a.damage);

                const bool offScreen = a.position.x < -50.f || a.position.x > 1050.f ||
                                       a.position.y < -50.f || a.position.y > 750.f;
                if (!hitBoss && !offScreen)
                    keptProjectiles.push_back(a);
            }
            playerArrows = std::move(keptProjectiles);

            // ----- Melee: press T to swing, one hit per swing -----
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T) && cleavecooldown == 0)
            {
                cleaveduration = melee.activeFrames;
                cleavecooldown = melee.cooldown;
                meleeHitThisSwing = false;
            }

            cleavin = cleaveduration > 0;
            if (cleaveduration > 0)
                cleaveduration--;
            if (cleavecooldown > 0)
                cleavecooldown--;

            if (cleavin)
            {
                cleave1.setPosition(playerCenter);
                cleave1.setRotation(sf::radians(std::atan2(cleaveAim.y, cleaveAim.x)));
                if (!meleeHitThisSwing && PlayerMeleeCheck(cleave1, boss1))
                {
                    bossHealth = std::max(0.f, bossHealth - melee.damage);
                    meleeHitThisSwing = true;
                }
            }

            // ----- Health bars -----
            PlayerHealthbarFront.setScale({1.f, playerHealth / playerMaxHealth});
            BossHealthbarFront.setScale({1.f, (bossHealth / bossMaxHealth) * 3.f});
            BossHealthbarBack.setScale({1.f, 3.f});

            // ----- Boss shooting -----
            shootTimer++;
            if (shootTimer >= shootInterval)
            {
                shootTimer = 0;
                const float length = std::sqrt(aim.x * aim.x + aim.y * aim.y);
                if (length > 0.f)
                    bullets.push_back({bossCenter, (aim / length) * bulletSpeed});
            }

            if (iFrames > 0)
                iFrames--;

        vector<enemyBullet> remaining;
        for (enemyBullet b : bullets)
        {
            b.position += b.velocity;
            projectile1.setPosition(b.position);

            const bool offScreen = b.position.x < -50.f || b.position.x > 1050.f ||
                                    b.position.y < -50.f || b.position.y > 750.f;
            const bool hitPlayer = player.getGlobalBounds()
                                    .findIntersection(projectile1.getGlobalBounds())
                                    .has_value();

            if (hitPlayer && iFrames == 0)
            {
                playerHealth = std::max(0.f, playerHealth - 7.5f);
                iFrames = 3;
            }

                if (!offScreen && !hitPlayer)
                remaining.push_back(b);
        }
        bullets = std::move(remaining);

            // ----- Drawing -----
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

            window.display();
            tickBossPhases++;
            break;
        }
        }
    }
    return 0;
}



