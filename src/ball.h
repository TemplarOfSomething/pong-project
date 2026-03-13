#pragma once
#include "SDL3/SDL_stdinc.h"
#include "cpup/canis.h"
#include "cpup/entity.h"
#include "cpup/io.h"
#include "cpup/math.h"
#include "cpup/scene.h"
#include "cpup/model.h"
#include "cpup/inputmanager.h"
#include "cpup/window.h"

#include <SDL3/SDL.h>

typedef struct {
    int rightScore;
    int leftScore;
} Ball;

Entity* l, *r, *b;
char score[] = "Score 0|0";
char* title = score;
const char* winTitle;

//some functions
Entity* SpawnBall(AppContext* _app, Entity* _entity);
void FauxBallUpdate(AppContext *_app, Entity *_entity);
void Victory(AppContext* _app, Entity* _entity);
void TrailBallUpdate(AppContext* _app, Entity* _entity);

void BallStart(AppContext* _app, Entity* _entity) {
    _entity->color = InitVector4(1.0f, 1.0f, 1.0f, 1.0f);
    _entity->transform.scale = InitVector3(32.0f, 32.0f, 1.0f);
    
    l = Find(&_app->scene, "lp");
    r = Find(&_app->scene, "rp");
}

void BallUpdate(AppContext* _app, Entity* _entity) {
    //easier data access
    //Ball* bData = (Ball*)_entity->data;
    
    if (GetKeyDown(_app, SDL_SCANCODE_P))
    {
        //SpawnBall(_app, _entity);
    }

    if (Vec2EqualsZero(_entity->velocity) && GetKeyDown(_app, SDL_SCANCODE_SPACE))
    {
        i32 startingDirection = rand() % 4;

        static Vector2 directions[4] = {
            (Vector2){0.72f, 0.72f},
            (Vector2){0.72f, -0.72f},
            (Vector2){-0.72f, -0.72f},
            (Vector2){-0.72f, -0.72f},
        };
        _entity->velocity = Vec2Mul(directions[startingDirection], 150.0f);
    }

    // check if ball is heading below the screen
    if (_entity->transform.position.y - _entity->transform.scale.y * 0.5f <= 0.0f && _entity->velocity.y < 0.0f)
        _entity->velocity.y *= -1.0f; 
    
    // check if ball is heading above the screen
    if (_entity->transform.position.y + _entity->transform.scale.y * 0.5f >= _app->windowHeight && _entity->velocity.y > 0.0f) {
        _entity->velocity.y *= -1.0f; 
    }

    // scoring
        // left (right paddle gets points)
    if (_entity->transform.position.x < 0.0f) {
        *(int*) (_entity->data) += 1;
        // cast void* to int*, dereference int* to get int, add 48 to get ASCII number then cast to char
        title[6] = (char) (*(int*)_entity->data)+48;
        SetWindowTitle(_app, winTitle);
        _entity->velocity = InitVector2(0.0f, 0.0f);
        _entity->transform.position = InitVector3(_app->windowWidth * 0.5f, _app->windowHeight * 0.5f, 0.0f);
        // win
        if (*(int *) (_entity->data) == 5) {
            Victory(_app, _entity);
        }
    }
        // right (left paddle gets points)
    if (_entity->transform.position.x > _app->windowWidth) {
        *(int *) (_entity->data+4) += 1;
        // add memory offset before cast
        title[8] = (char) (*(int*)(_entity->data+4))+48;
        SetWindowTitle(_app, winTitle);
        _entity->velocity = InitVector2(0.0f, 0.0f);
        _entity->transform.position = InitVector3(_app->windowWidth * 0.5f, _app->windowHeight * 0.5f, 0.0f);
        // win
        if (*(int *) (_entity->data+4) == 5) {
            Victory(_app, _entity);
        }
    }

    // collision
        // right paddle
    if (_entity->transform.position.x + _entity->transform.scale.x * 0.5f >= 
        r->transform.position.x - _entity->transform.scale.x * 0.5) {
        if ((_entity->transform.position.y + _entity->transform.scale.y * 0.5f >=
        r->transform.position.y - _entity->transform.scale.y) * 0.5 && 
        (_entity->transform.position.y - _entity->transform.scale.y * 0.5f <=
        r->transform.position.y + _entity->transform.scale.y * 0.5)) {
            _entity->velocity.x *= -1.1f;
            _entity->color = InitVector4(0.0f, 0.0f, 1.0f, 1.0f);
            //"bounce"
            r->transform.scale.x *= 0.75f;
            r->transform.scale.y *= 1.25f;
            *(float *)r->data = _app->deltaTime + 5e-8;
        }
    }
        // left paddle
    if (_entity->transform.position.x - _entity->transform.scale.x * 0.5f <= 
        l->transform.position.x + _entity->transform.scale.x * 0.5) {
        if ((_entity->transform.position.y + _entity->transform.scale.y * 0.5f >=
        l->transform.position.y - _entity->transform.scale.y) * 0.5 && 
        (_entity->transform.position.y - _entity->transform.scale.y * 0.5f <=
        l->transform.position.y + _entity->transform.scale.y * 0.5)) {
            _entity->velocity.x *= -1.1f;
            _entity->color = InitVector4(1.0f, 0.0f, 0.0f, 1.0f);
            // "bounce"
            l->transform.scale.x *= 0.75f;
            l->transform.scale.y *= 1.25f;
            *(float *)l->data = _app->deltaTime + 5e-8;
        }
    }

    Vector3 delta = Vec2ToVec3(Vec2Mul(_entity->velocity, _app->deltaTime));
    _entity->transform.position = Vec3Add(_entity->transform.position, delta);
}

void BallDraw(AppContext* _app, Entity* _entity) {
    Matrix4 transform = IdentityMatrix4(); // the order is important
    Mat4Translate(&transform, _entity->transform.position);
    Mat4Rotate(&transform, _entity->transform.rotation * DEG2RAD, InitVector3(0.0f, 0.0f, 1.0f));
    Mat4Scale(&transform, InitVector3(_entity->transform.scale.x, _entity->transform.scale.y, _entity->transform.scale.z));

    BindShader(_entity->shaderId);

    ShaderSetFloat(_entity->shaderId, "TIME", _app->time);
    ShaderSetMatrix4(_entity->shaderId, "VIEW", _app->view);
    ShaderSetMatrix4(_entity->shaderId, "PROJECTION", _app->projection);

    ShaderSetVector4(_entity->shaderId, "COLOR", _entity->color);
    ShaderBindTexture(_entity->shaderId, _entity->image->id, "MAIN_TEXTURE", 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    ShaderSetMatrix4(_entity->shaderId, "TRANSFORM", transform);
    DrawModel(*_entity->model);

    UnBindShader();
}

void BallOnDestroy(AppContext* _app, Entity* _entity) {

}

void FauxBallUpdate(AppContext* _app, Entity* _entity) {
    Vector3 delta = Vec2ToVec3(Vec2Mul(_entity->velocity, _app->deltaTime));
    _entity->transform.position = Vec3Add(_entity->transform.position, delta);
}

void TrailBallUpdate(AppContext* _app, Entity* _entity) {
    b = Find(&_app->scene, "b");
    _entity->color = b->color;
    _entity->transform.scale.x -= (32.0f) * _app->deltaTime;
    _entity->transform.scale.y -= (32.0f) * _app->deltaTime;

    if (_entity->transform.scale.x <= 0.0f) {
        _entity->transform.position = b->transform.position;
        _entity->transform.scale = b->transform.scale;
    }
}

void Victory(AppContext* _app, Entity* _entity) {
    for (int i = 0; i < 100; i++) {
        Entity* temp = SpawnBall(_app, _entity);
        temp->name="t";
        temp->transform.scale = InitVector3(32.0f, 32.0f, 1.0f);
        temp->color = _entity->color;
        temp->transform.position = InitVector3(random_float(0.0f, _app->windowWidth), _app->windowHeight - 20.0f, 0.01f);
        temp->velocity = InitVector2(random_float(-1.5f, 1.5f), random_float(-200.0f, -50.0f));
        temp->Update = FauxBallUpdate;
        // overrides color if unchanged
        temp->Start = NULL;
    }
}

Entity* SpawnBall(AppContext* _app, Entity* _entity) {
    Entity* ball = Spawn(&(_app->scene));
    ball->transform.position = InitVector3(_app->windowWidth * 0.5f, _app->windowHeight * 0.5f, 0.0f);
    ball->data = calloc(1, sizeof(Ball));
    ball->image = _entity->image;
    ball->model = _entity->model;
    ball->shaderId = _entity->shaderId;
    ball->Start = BallStart;
    ball->Update = BallUpdate;
    ball->Draw = BallDraw;
    ball->OnDestroy = BallOnDestroy;
    return ball;
}