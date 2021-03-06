//===========================================================
// File: TerrainHandler.cpp	
// StudentName: Per-Morten Straume                          
//                                                          
// Exam 2015: IMT-2531 Graphics Programming Exam.                                
//===========================================================
#include "TerrainHandler.h"
#include <algorithm>
#include <iostream>
#include <random>

#include "Consts.h"

TerrainHandler::TerrainHandler(Renderer& renderer, const HeightMap& heightMap) noexcept
    : _renderer(renderer)
    , _downPour(maxRain)
{
    createTerrain(heightMap);
    hideUndrawableTerrain();
    createDrawableSceneList();
    createDownPour();

    std::vector<glm::vec3> positions;
    std::vector<GLuint> textureIndices;
    positions.reserve(_drawableSceneObjects.size());
    textureIndices.reserve(_drawableSceneObjects.size());
    for (const auto& object : _drawableSceneObjects)
    {
        positions.emplace_back(object->getPosition());
        textureIndices.emplace_back(object->getTextureIndex());
    }

    _renderer.createStaticLandscape("static", positions, textureIndices);
}

TerrainHandler::~TerrainHandler() noexcept
{
    for (std::size_t i = 0; i < _sceneObjects.size(); ++i)
    {
        for (std::size_t j = 0; j < _sceneObjects[i].size(); ++j)
        {
            for (std::size_t k = 0; k < _sceneObjects[i][j].size(); ++k)
            {
                delete _sceneObjects[i][j][k];
            }
        }
    }
    for (std::size_t i = 0; i < _downPour.size(); ++i)
    {
        delete _downPour[i];
    }
    delete _cloud;
}

void TerrainHandler::update(float deltaTime) noexcept
{
//#define ORIGINAL
#ifdef ORIGINAL
    for (auto& object : _drawableSceneObjects)
    {
        object->update(deltaTime);
        object->draw();
    }
#endif
    if (_isRaining || _isSnowing)
    {
        updateDownPour(deltaTime);
    }

    glm::mat4 mat(1);

#ifndef ORIGINAL
    _renderer.render(Renderer::simpleShader, "static", Renderer::groundTexture, mat, glm::vec4( 1.0f, 1.0f, 1.0f , 1.0f), 0);
#endif
}

void TerrainHandler::addCube(std::size_t xIndex, std::size_t zIndex) noexcept
{
    auto size = _sceneObjects[xIndex][zIndex].size();
    glm::vec3 newPosition;
    if (size > 0)
    {
        newPosition = _sceneObjects[xIndex][zIndex][size - 1]->getPosition();
        newPosition.y += SceneObject::cubeSize;
    }
    else
    {
        newPosition = { xIndex * SceneObject::cubeSize, 0.0f, zIndex * SceneObject::cubeSize };
    }
    _sceneObjects[xIndex][zIndex].push_back(new SceneObject(_renderer,
                                                            Renderer::regularShader,
                                                            Renderer::cubeMesh,
                                                            Renderer::groundTexture,
                                                            SceneObject::dirtTexture,
                                                            newPosition));
    applyCorrectTextures();
    hideUndrawableTerrain();
    createDrawableSceneList();
}

void TerrainHandler::deleteCube(std::size_t xIndex, std::size_t zIndex) noexcept
{
    std::size_t size = _sceneObjects[xIndex][zIndex].size();
    if (size > 0)
    {
        delete _sceneObjects[xIndex][zIndex][size - 1];

        _sceneObjects[xIndex][zIndex].pop_back();
        applyCorrectTextures();
        hideUndrawableTerrain();
        createDrawableSceneList();
    }
}

void TerrainHandler::switchToNextTextureSet() noexcept
{
    _baseTexture += Consts::NUMBEROFROWSINATLAS;
    if (_baseTexture >= Consts::NUMBEROFROWSINATLAS * Consts::NUMBEROFROWSINATLAS)
    {
        _baseTexture = 0;
    }
    applyCorrectTextures();
}

void TerrainHandler::toggleRain() noexcept
{
    if (_isSnowing)
    {
        _isSnowing = false;
    }
    _isRaining = !_isRaining;
    _cloud->setVisible(_isRaining);
    applyCorrectTextures();
    makeRain();
}

void TerrainHandler::toggleSnow() noexcept
{
    if (_isRaining)
    {
        _isRaining = false;
    }
    _isSnowing = !_isSnowing;
    _cloud->setVisible(_isSnowing);
    applyCorrectTextures();
    makeSnow();
}

void TerrainHandler::updateDownPour(float deltaTime) noexcept
{
    if (_cloud)
    {
        _cloud->update(deltaTime);
        _cloud->draw();
    }

    for (std::size_t i = 0; i < _downPour.size(); ++i)
    {
        if (_downPour[i]->getPosition().y <= 0.0f)
        {
            _downPour[i]->setVisible(false);
        }
        if (!_downPour[i]->isVisible())
        {
            startDownPour(_downPour[i], deltaTime);
        }
        auto newPosition = _downPour[i]->getPosition();
        newPosition.y -= downPourFallSpeed * deltaTime;
        if (newPosition.y * _downPour[i]->getScale().y <= 0.0f)
        {
            _downPour[i]->setVisible(false);
        }
        _downPour[i]->setPosition(newPosition);
        _downPour[i]->setUpdated();

        if (_downPour[i]->isVisible())
        {
            _downPour[i]->update(deltaTime);
            _downPour[i]->draw();
        }

    }

}

void TerrainHandler::createTerrain(const HeightMap& heightMap) noexcept
{
    float highestValue = 0;
    _sceneObjects.resize(heightMap.size());
    for (std::size_t i = 0; i < heightMap.size(); ++i)
    {
        _sceneObjects[i].resize(heightMap[i].size());
        for (std::size_t j = 0; j < heightMap[i].size(); ++j)
        {
            for (int k = 0; k < heightMap[i][j]; ++k)
            {
                SceneObject* object = new SceneObject(_renderer,
                                                      Renderer::regularShader,
                                                      Renderer::cubeMesh,
                                                      Renderer::groundTexture,
                                                      SceneObject::dirtTexture,
                                                      glm::vec3(i * SceneObject::cubeSize, k * SceneObject::cubeSize, j * SceneObject::cubeSize));
                _sceneObjects[i][j].push_back(object);

                if (heightMap[i][j] > highestValue)
                {
                    highestValue = static_cast<float>(heightMap[i][j]);
                }
            }
        }
    }
    highestValue += highestValue * 0.5f;
    applyCorrectTextures();
    _cloud = new SceneObject(_renderer,
                             Renderer::nonScalingWLight,
                             Renderer::cubeMesh,
                             Renderer::groundTexture,
                             SceneObject::cloudTexture,
                             glm::vec3((heightMap.size() * SceneObject::cubeSize) / 2, highestValue * SceneObject::cubeSize, (heightMap[0].size() * SceneObject::cubeSize) / 2),
                             glm::vec3(heightMap.size(), SceneObject::cubeSize / 4, heightMap[0].size()));
}

void TerrainHandler::hideUndrawableTerrain() noexcept
{
    constexpr std::size_t maxSizeT = std::numeric_limits<std::size_t>::max();

    for (std::size_t i = 0; i < _sceneObjects.size(); ++i)
    {
        for (std::size_t j = 0; j < _sceneObjects[i].size(); ++j)
        {
            for (std::size_t k = _sceneObjects[i][j].size() - 1; k != maxSizeT; --k)
            {
                // Draw and update the cubes which should be visible
                bool hidden = false;
                if (i != 0 && i != _sceneObjects.size() - 1 &&
                    j != 0 && j != _sceneObjects[i].size() - 1 &&
                    k != 0 && k != _sceneObjects[i][j].size() - 1)
                {
                    hidden = true;
                    hidden = hidden && (k < _sceneObjects[i + 1][j + 0].size()); // Right
                    hidden = hidden && (k < _sceneObjects[i + 0][j + 1].size()); // Up
                    hidden = hidden && (k < _sceneObjects[i - 1][j + 0].size()); // Left
                    hidden = hidden && (k < _sceneObjects[i + 0][j - 1].size()); // Down
                }
                if (!hidden)
                {
                    _sceneObjects[i][j][k]->setVisible(true);
                }
            }
        }
    }
}

void TerrainHandler::createDrawableSceneList() noexcept
{
    SceneObjectList temp;
    for (std::size_t i = 0; i < _sceneObjects.size(); ++i)
    {
        for (std::size_t j = 0; j < _sceneObjects[i].size(); ++j)
        {
            for (std::size_t k = 0; k < _sceneObjects[i][j].size(); ++k)
            {
                temp.push_back(_sceneObjects[i][j][k]);
            }
        }
    }

    auto tempEnd = std::partition(temp.begin(), temp.end(), [](auto& object)
                                  {
                                      return object->isVisible();
                                  });

    // Copy into the list of drawable elements
    _drawableSceneObjects.clear();
    std::copy(temp.begin(), tempEnd, std::back_inserter(_drawableSceneObjects));

    // Sort it based on the texture offset so we don't have to send that uniform so often
    std::sort(_drawableSceneObjects.begin(), _drawableSceneObjects.end(), [](auto& a, auto& b)
              {
                  return (a->getTextureIndex() < b->getTextureIndex());
              });
}

void TerrainHandler::applyCorrectTextures() noexcept
{
    for (std::size_t i = 0; i < _sceneObjects.size(); ++i)
    {
        for (std::size_t j = 0; j < _sceneObjects[i].size(); ++j)
        {
            for (std::size_t k = 0; k < _sceneObjects[i][j].size(); ++k)
            {
                GLuint textureIndex = 0;
                if (k < shallowWaterLevel)
                {
                    textureIndex = SceneObject::deepWaterTexture;
                }
                else if (k < dirtLevel)
                {
                    textureIndex = SceneObject::shallowWaterTexture;
                }
                else if (k < grassLevel)
                {
                    textureIndex = SceneObject::dirtTexture;
                }
                else if (k < snowLevel)
                {
                    textureIndex = SceneObject::grassTexture;
                }
                else
                {
                    textureIndex = SceneObject::snowTexture;
                }

                //make sure borders of the map get the dirt texture
                if ((i == 0 || i == _sceneObjects.size() - 1 ||
                     j == 0 || j == _sceneObjects[i].size() - 1) &&
                    k < _sceneObjects[i][j].size() - 1 && k > dirtLevel && (k <= snowLevel))
                {
                    textureIndex = SceneObject::dirtTexture;
                }
                if (k < _sceneObjects[i][j].size() - 1 && k > dirtLevel && (k <= snowLevel))
                {
                    textureIndex = SceneObject::dirtTexture;
                }

                if (_isSnowing)
                {
                    textureIndex = SceneObject::snowTexture;
                }
                if (_isSnowing && k < shallowWaterLevel)
                {
                    textureIndex = SceneObject::shallowWaterTexture;
                }
                _sceneObjects[i][j][k]->setTextureIndex(textureIndex + _baseTexture);
            }
        }
    }
}

void TerrainHandler::createDownPour() noexcept
{
    glm::vec3 position{ 0.0f,0.0f,0.0f };
    glm::vec3 scale{ downPourSize, downPourSize, downPourSize };
    for (std::size_t i = 0; i < _downPour.size(); ++i)
    {
        _downPour[i] = new SceneObject(_renderer,
                                       Renderer::nonScalingWLight,
                                       Renderer::cubeMesh,
                                       Renderer::groundTexture,
                                       SceneObject::deepWaterTexture,
                                       position,
                                       scale);
    }
}

void TerrainHandler::makeRain() noexcept
{
    for (std::size_t i = 0; i < _downPour.size(); ++i)
    {
        _downPour[i]->setTextureIndex(SceneObject::deepWaterTexture);
    }
}
void TerrainHandler::makeSnow() noexcept
{
    for (std::size_t i = 0; i < _downPour.size(); ++i)
    {
        _downPour[i]->setTextureIndex(SceneObject::snowTexture);
    }
}

void TerrainHandler::startDownPour(SceneObject* drop, float deltaTime) noexcept
{
    static float timeToNextDrop;
    timeToNextDrop += deltaTime;

    if (timeToNextDrop >= timeBetweenDrops)
    {
        auto rainDropStartPosition = _cloud->getPosition();

        int lowerXCoord = static_cast<int>(_cloud->getPosition().x - _cloud->getScale().x * SceneObject::cubeSize / 2);
        int upperXCoord = static_cast<int>(_cloud->getPosition().x + _cloud->getScale().x * SceneObject::cubeSize / 2);
        int lowerZCoord = static_cast<int>(_cloud->getPosition().z - _cloud->getScale().z * SceneObject::cubeSize / 2);
        int upperZCoord = static_cast<int>(_cloud->getPosition().z + _cloud->getScale().z * SceneObject::cubeSize / 2);

        std::random_device randomizer;
        std::default_random_engine engine(randomizer());
        std::uniform_int_distribution<int> distributionX(lowerXCoord, upperXCoord);
        std::uniform_int_distribution<int> distributionZ(lowerZCoord, upperZCoord);

        rainDropStartPosition.x = static_cast<float>(distributionX(engine));
        rainDropStartPosition.z = static_cast<float>(distributionZ(engine));

        drop->setVisible(true);
        drop->setPosition(rainDropStartPosition);

        timeToNextDrop = 0.0f;
    }
}

