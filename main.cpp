#include <SFML/Graphics.hpp>
#include "src/imgui/imgui-SFML.h"
#include "src/imgui/imgui.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <algorithm>

struct myBool{
    bool value;
};

struct CustomSFMLShape{
    std::shared_ptr<sf::Shape> m_shape; 
    char m_name[100];
    float speedX;
    float speedY;
};

sf::RenderWindow window;
sf::Font myFont;
sf::Text t;
std::vector<sf::Text> texts;
std::vector<CustomSFMLShape> shapes;
std::vector<sf::CircleShape> dots;


int width;
int height;
int maxCharHeight;

std::vector<std::vector<float>> colors;
char shapesNames[1024];
std::vector<myBool> isVisible;
std::vector<float> scale;
int selectedItemIndex = -1;

void initSFML();
void checkOutOfBounds();

int main(){
    initSFML();

    window.setFramerateLimit(60);
    
    // init imgui
    ImGui::SFML::Init(window); 
    sf::Clock deltaClock;
    ImGui::GetStyle().ScaleAllSizes(1.0f);

    selectedItemIndex = (shapes.size() == 0 ? -1: 0);
    // Game loop
    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event)){
            ImGui::SFML::ProcessEvent(window, event);

            if(event.type == sf::Event::Closed){
                window.close();
            }

            if(event.key.code == sf::Keyboard::Escape){
                window.close();
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        // imgui ui
        if(selectedItemIndex != -1){
            ImGui::Begin("Shapes Editor");
            ImGui::Checkbox("Show Shape", &isVisible[selectedItemIndex].value);
            ImGui::Combo("Shapes", &selectedItemIndex, shapesNames);
            ImGui::SliderFloat("Scale", &scale[selectedItemIndex], 0.f, 4.0f);
            ImGui::SliderFloat2("Velocity", &(shapes[selectedItemIndex].speedX), -8.0f, 8.0f);
            ImGui::ColorEdit3("Color", colors[selectedItemIndex].data());
            ImGui::InputText("Shape's name", shapes[selectedItemIndex].m_name, 100);
            ImGui::End();
        }

        // update the currently-selected shape
        if(selectedItemIndex != -1){
            shapes[selectedItemIndex].m_shape->setFillColor(sf::Color(colors[selectedItemIndex][0] * 255, colors[selectedItemIndex][1] * 255,
                 colors[selectedItemIndex][2] * 255));
            shapes[selectedItemIndex].m_shape->setScale(scale[selectedItemIndex], scale[selectedItemIndex]);
        }

        // update all shapes
        for(size_t i = 0; i < shapes.size(); ++i){
            shapes[i].m_shape->move(shapes[i].speedX, shapes[i].speedY);

            sf::Vector2f position = shapes[i].m_shape->getPosition();

            float offset_y = texts[i].getGlobalBounds().top - texts[i].findCharacterPos(0).y;
            float offset_x = texts[i].getGlobalBounds().left - texts[i].findCharacterPos(0).x;
            

            float max_char_height = 0;

            // need to precompute in case string has changed
            for(size_t index = 0; index < texts[i].getString().getSize(); ++index){
                sf::FloatRect box =  myFont.getGlyph(texts[i].getString()[index], 18, false).bounds;

                if(box.height > max_char_height){
                    max_char_height = box.height;
                }
            }

            texts[i].setPosition(position.x + shapes[i].m_shape->getGlobalBounds().width/2.0f - texts[i].getGlobalBounds().width/2.0f - offset_x,
                         position.y + shapes[i].m_shape->getGlobalBounds().height/2.0f - offset_y - max_char_height/2.0f);
            
            texts[i].setString(shapes[i].m_name);

            dots[i].setPosition(position.x + shapes[i].m_shape->getGlobalBounds().width/2.0f - dots[i].getRadius(),
                                 position.y + shapes[i].m_shape->getGlobalBounds().height/2.0f - dots[i].getRadius());
        }
        checkOutOfBounds(); // check if any of the shapes got out of bounds



        window.clear();
        for(size_t i = 0; i < shapes.size(); ++i){
            if(!isVisible[i].value)
                continue;
            window.draw(*(shapes[i].m_shape));
            window.draw(texts[i]);
            window.draw(dots[i]);
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}

void initSFML(){
    std::ifstream in("config.txt");

    if(!in.is_open()){
        std::cerr << "could not open the config file\n";
        exit(-1);
    }

    std::string mode;

    while(in >> mode){
        if(mode == "Window"){
            in >> width >> height;
            window.create(sf::VideoMode(width, height), "Hello SFML");
        }
        else if(mode == "Fonts"){
            std::string filepath;
            unsigned int fontSize, r, g, b;

            in >> filepath;
            if(!myFont.loadFromFile(filepath)){
                std::cerr << "could not open the font file\n";
                exit(-1);
            }
            in >> fontSize >> r >> g >> b;
            t.setFont(myFont);
            t.setCharacterSize(fontSize);
            t.setFillColor(sf::Color(r, g, b));
        }
        else{
            unsigned int r, g, b;
            float xPos, yPos, xSpeed, ySpeed, radius, width, height;

            std::string shapeName;
            in >> shapeName;
            in >> xPos >> yPos >> xSpeed >> ySpeed >> r >> g >> b;

            if(mode == "Circle"){
                in >> radius;

                sf::CircleShape circle(radius);
                circle.setFillColor(sf::Color(r, g, b));
                circle.setPosition(xPos, yPos);

                CustomSFMLShape ccs;
                ccs.m_shape = std::make_shared<sf::CircleShape>(circle);
                ccs.speedX = xSpeed;
                ccs.speedY = ySpeed;
                
                for(int i = 0; i < 100; ++i)
                    ccs.m_name[i] = '\0';

                for(size_t i = 0; i < shapeName.size(); ++i){
                    ccs.m_name[i] = shapeName[i];
                }

                shapes.push_back(ccs);
            }
            else if(mode == "Rectangle"){
                in >> width >> height;

                sf::Vector2f size = {width, height};
                sf::RectangleShape rectangle(size);
                rectangle.setFillColor(sf::Color(r, g, b));
                rectangle.setPosition(xPos, yPos);

                CustomSFMLShape ccs;
                ccs.m_shape = std::make_shared<sf::RectangleShape>(rectangle);
                ccs.speedX = xSpeed;
                ccs.speedY = ySpeed;

                for(int i = 0; i < 100; ++i)
                    ccs.m_name[i] = '\0';
                for(size_t i = 0; i < shapeName.size(); ++i){
                    ccs.m_name[i] = shapeName[i];
                }

                shapes.push_back(ccs);
            }
            else{
                std::cerr << "undefined shape\n";
                exit(-1);
            }
        }
    }

    // init texts vector
    texts.resize(shapes.size());
    for(int i = 0; i < texts.size(); ++i){
        texts[i] = t;
        texts[i].setString(shapes[i].m_name);
        // texts[i].setOrigin(texts[i].getGlobalBounds().width/2, texts[i].getGlobalBounds().height/2);
    }


    colors.resize(shapes.size());
    for(size_t i = 0; i < shapes.size(); ++i){
        colors[i].resize(3);
        sf::Color c = shapes[i].m_shape->getFillColor();
        colors[i][0] = c.r/255.f;
        colors[i][1] = c.g/255.f;
        colors[i][2] = c.b/255.f;
    }

    scale.resize(shapes.size(), 1);

    isVisible.resize(shapes.size());
    std::for_each(isVisible.begin(), isVisible.end(), [](myBool& mb) {mb.value = true;});

    dots.resize(shapes.size());
    for_each(dots.begin(), dots.end(), [](sf::CircleShape& c){c.setRadius(3.0f);});

    // init shapes names for imgui 
    int index = 0;
    for(const auto& shape: shapes){
        int i = 0;
        while(shape.m_name[i] != '\0'){
            shapesNames[index++] = shape.m_name[i++];
        }
        shapesNames[index++] = '\0';
    }
}

void checkOutOfBounds(){
    for(auto& shape: shapes){
        if(shape.m_shape->getPosition().x > width - shape.m_shape->getGlobalBounds().width){
            shape.m_shape->setPosition(width - shape.m_shape->getGlobalBounds().width,
            shape.m_shape->getPosition().y);    // avoid out of bounds glitch when holding the cursor
            shape.speedX = -shape.speedX;     // enough for typical bouncing off bounds
        }
        else if(shape.m_shape->getPosition().x < 0){
            shape.m_shape->setPosition(0, shape.m_shape->getPosition().y);
            shape.speedX = -shape.speedX;
        }
        else if(shape.m_shape->getPosition().y > height - shape.m_shape->getGlobalBounds().height){
            shape.m_shape->setPosition(shape.m_shape->getPosition().x,
                height - shape.m_shape->getGlobalBounds().height);
            shape.speedY = -shape.speedY;
        }
        else if(shape.m_shape->getPosition().y < 0){
            shape.m_shape->setPosition(shape.m_shape->getPosition().x, 0);
            shape.speedY = -shape.speedY;
        }
    }
}