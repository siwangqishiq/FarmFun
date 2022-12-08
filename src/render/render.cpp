#include "render.hpp"
#include "application.hpp"
#include "log.hpp"
#include "command.hpp"
#include "vram.hpp"
#include "texture.hpp"
#include "resource/asset_manager.hpp"
#include "../libjson/json.hpp"
#include "glheader.hpp"

void RenderEngine::render(){
    // glClearColor(0.0f , 1.0f , 0.0f , 1.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_DEPTH_TEST);
    
    //gl render
    for(auto &cmd : renderCommandList_){
        cmd->runCommands();
        cmd->used = false;
    }
    
    //clear cmd list
    clearRenderCommands();
    VRamManager::getInstance().onPostRender();
}

void RenderEngine::free(){
    VRamManager::getInstance().clear();
    ShaderManager::getInstance()->clear();
    TextureManager::getInstance()->clear();
}

void RenderEngine::clearRenderCommands(){
    renderCommandList_.clear();
}

void RenderEngine::onScreenResize(){
    resetNormalMat(appContext_->viewWidth_ , appContext_->viewHeight_);
    glViewport(0 , 0 , appContext_->screenWidth_ , appContext_->screenHeight_);
}

void RenderEngine::init(){
    Logi(TAG , "render engine init start");

    loadTextRenderResource();//text render init
    // Logi(TAG , "render engine init end");

}

void RenderEngine::loadTextRenderResource(){
    Logi(TAG , "render init loadTextRenderResource");
    textRenderHelper_ = std::make_shared<TextRenderHelper>();
    textRenderHelper_->loadRes(*this);
}

void RenderEngine::resetNormalMat(float w , float h){
    Logi(TAG , "reset normal matrix %f , %f" , w , h);
    normalMatrix_[0][0] = 2.0f / w;
    normalMatrix_[0][1] = 0.0f;
    normalMatrix_[0][2] = 0.0f;

    normalMatrix_[1][0] = 0.0f;
    normalMatrix_[1][1] = 2.0f / h;
    normalMatrix_[1][2] = 0.0f;

    normalMatrix_[2][0] = -1.0f;
    normalMatrix_[2][1] = -1.0f;
    normalMatrix_[2][2] =  1.0f;
}

void RenderEngine::submitRenderCommand(std::shared_ptr<RenderCommand> cmd){
    renderCommandList_.push_back(cmd);
}

void RenderEngine::renderText(std::wstring text , 
        float left , float bottom , TextPaint &paint){
    auto cmd = fetchTextRenderCommand(this);
    cmd->putParams(text , left , bottom , paint);
    submitRenderCommand(cmd);
}

void RenderEngine::renderText(std::wstring text , Rect &showRect , TextPaint &paint){
    if(text.empty()){
        return;
    }

    auto cmd = fetchTextRenderCommand(this);
    cmd->putTextParamsByRectLimit(text , showRect , paint);
    submitRenderCommand(cmd);
}

std::shared_ptr<TextRenderCommand> RenderEngine::fetchTextRenderCommand(RenderEngine *engine){
    // new a new instace 
    // later use pool to reuse
    auto newCmd = 
        std::make_shared<TextRenderCommand>(this);
    newCmd->used = true;
    return newCmd;
}

void TextRenderHelper::loadRes(RenderEngine &engine){
    textRenderShader_ = ShaderManager::getInstance()->loadAssetShader("text_render" 
                                , "shader/render_text_vert.glsl"
                                , "shader/render_text_frag.glsl");
    
    buildTextCharConfig();
}

std::shared_ptr<CharInfo> TextRenderHelper::findCharInfo(wchar_t &ch){
    return charInfoMaps_[ch];
}

//读取字符配置
void TextRenderHelper::buildTextCharConfig(){

    std::wstring charConfigStr = AssetManager::getInstance()->readTextFile("text/char_config.json");

    JsonParser parser;
    auto configJson = parser.parseJsonObject(charConfigStr);
    auto charJsonList = configJson->getJsonArray("list");
    Logi("text_render" , "charJsonList size : %d" , charJsonList->size());

    charInfoMaps_.clear();

    for(int i = 0 ;  i< charJsonList->size();i++){
        auto itemJson = charJsonList->getJsonObject(i);
        std::shared_ptr<CharInfo> info = std::make_shared<CharInfo>();
        info->value = itemJson->getString("value");
        info->width = itemJson->getInt("width");
        info->height = itemJson->getInt("height");
        auto texCoordsArray = itemJson->getJsonArray("texCoords");
        for(int i = 0 ; i < 4; i++){
            info->textureCoords[i] = texCoordsArray->getFloat(i);
        }//end for i

        //load texture
        // auto textureName = itemJson->getString("texture");
        std::string textureName = ToByteString(itemJson->getString("texture"));
        // Logi("text_render" , "textureName coord: %f , %f , %f , %f" , 
        //     info->textureCoords[0] ,info->textureCoords[1],
        //     info->textureCoords[2] ,info->textureCoords[3] );
        // Logi("text_render" , "texture value size %d" , info->value.size());
        // Logi("text_render" , "texture name  %s" , textureName.c_str());
        auto textureInfo = TextureManager::getInstance()->acquireTexture("text/" + textureName);
        info->textureId = textureInfo->textureId;

        mainTextureId_ = info->textureId;
        // // Logi("text_render" , "textureId : %d" , info->textureId);
        charInfoMaps_.insert(std::make_pair<>(info->value[0] , info));
    }//end for i

    Logi("text_render" , "charInfoMaps size : %d" , charInfoMaps_.size());
    Logi("text_render" , "texture : %s" , 
            TextureManager::getInstance()->allTextureInfos().c_str());

    addSpecialTextCharInfo();
}

//添加特殊字符配置
void TextRenderHelper::addSpecialTextCharInfo(){
    auto blankCharInfo = std::make_shared<CharInfo>();
    blankCharInfo->textureId = 0;
    blankCharInfo->value = L" ";
    blankCharInfo->width = SPACE_WIDTH;
    blankCharInfo->height = 0.0f;
    blankCharInfo->textureCoords[0] = 0.0f;
    blankCharInfo->textureCoords[1] = 0.0f;
    blankCharInfo->textureCoords[2] = 0.0f;
    blankCharInfo->textureCoords[3] = 0.0f;
    charInfoMaps_.insert(std::make_pair<>(blankCharInfo->value[0] , blankCharInfo));
}