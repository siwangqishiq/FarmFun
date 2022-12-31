#pragma once

#include "application.hpp"

class RenderEngine;
class Image;

class TestDemo{
public:
    TestDemo(Application *app){
        appContext = app;
    }

    void init();

    void tick();

    void dispose();

private:
    Application *appContext;

    int viewWidth_;
    int viewHeight_;

    std::shared_ptr<RenderEngine> renderEngine_;

    float mRadius = 1.0f;

    std::shared_ptr<Image> testImage;

    void testAudio();
    
    void testRender1();
    void testRender2();
    void testRender3();
    void testRender4();
    void testRenderRoundRect();
    void testRenderTableTennis();

    void testRenderSprite1();
};