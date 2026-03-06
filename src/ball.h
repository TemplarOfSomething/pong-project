#pragma once
#include "cpup/canis.h"
#include "cpup/io.h"
#include "cpup/math.h"
#include "cpup/scene.h"
#include "cpup/model.h"
#include "cpup/inputmanager.h"

#include <SDL3/SDL.h>

typedef struct {
    int leftScore;
    int rightScore;
    Entity* leftPaddle;
    Entity* rightPaddle;
} Ball;

Entity* SpawnBall(AppContext* _app, Entity* _entity);
void fauxBallUpdate(AppContext *_app, Entity *_entity);

void BallStart(AppContext* _app, Entity* _entity) {
    _entity->color = InitVector4(1.0f, 1.0f, 1.0f, 1.0f);
    _entity->transform.scale = InitVector3(32.0f, 32.0f, 1.0f);
}

void BallUpdate(AppContext* _app, Entity* _entity) {
    //lazy method
    Entity* l = Find(&_app->scene, "lp");
    Entity* r = Find(&_app->scene, "rp");

    if (GetKeyDown(_app, SDL_SCANCODE_P))
    {
        SpawnBall(_app, _entity);
    }

    if (Vec2EqualsZero(_entity->velocity) && GetKeyDown(_app, SDL_SCANCODE_SPACE))
    {
        i32 startingDirection = rand() % 4;

        static Vector2 directions[4] = {
            (Vector2){0.72f, 0.5f},
            (Vector2){0.72f, -0.5f},
            (Vector2){-0.72f, 0.5f},
            (Vector2){-0.72f, -0.5f},
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

    // scoring (pointer "fun")
        // left
    if (_entity->transform.position.x < 0.0f) {
        *(int *) (_entity->data) += 1;
        _entity->velocity = InitVector2(0.0f, 0.0f);
        _entity->transform.position = InitVector3(_app->windowWidth * 0.5f, _app->windowHeight * 0.5f, 0.0f);
        if (*(int *) (_entity->data) == 1) {
            for (int i = 0; i < 100; i++) {
                Entity* temp = SpawnBall(_app, _entity);
                temp->transform.scale = InitVector3(32.0f, 32.0f, 1.0f);
                temp->color = InitVector4(0.0f, 0.0f, 1.0f, 1.0f);
                temp->transform.position = InitVector3(random_float(0.0f, _app->windowWidth), _app->windowHeight - 20.0f, 0.0f);
                temp->velocity = InitVector2(random_float(-1.5f, 1.5f), random_float(-200.0f, -50.0f));
                temp->Update = fauxBallUpdate;
                // overrides color if unchanged
                temp->Start = NULL;
            }
        }
    }
        // right
    if (_entity->transform.position.x > _app->windowWidth) {
        *(int *) (_entity->data+4) += 1;
        _entity->velocity = InitVector2(0.0f, 0.0f);
        _entity->transform.position = InitVector3(_app->windowWidth * 0.5f, _app->windowHeight * 0.5f, 0.0f);
        if (*(int *) (_entity->data+4) == 5) {
            
        }
    }

    //for struct modification do trolling:
    //*(type*) ptr + offset = value
    //access should be type var = * ptr + offsett???

    // collision
        // right paddle
    if (_entity->transform.position.x + _entity->transform.scale.x * 0.5f >= 
        r->transform.position.x - _entity->transform.scale.x * 0.5) {
        if ((_entity->transform.position.y + _entity->transform.scale.y * 0.5f >=
        r->transform.position.y - _entity->transform.scale.y) * 0.5 && 
        (_entity->transform.position.y - _entity->transform.scale.y * 0.5f <=
        r->transform.position.y + _entity->transform.scale.y * 0.5)) {
            _entity->velocity.x *= -1.0f;
            _entity->color = InitVector4(0.0f, 0.0f, 1.0f, 1.0f);
        }
    }
        // left paddle
    if (_entity->transform.position.x - _entity->transform.scale.x * 0.5f <= 
        l->transform.position.x + _entity->transform.scale.x * 0.5) {
        if ((_entity->transform.position.y + _entity->transform.scale.y * 0.5f >=
        l->transform.position.y - _entity->transform.scale.y) * 0.5 && 
        (_entity->transform.position.y - _entity->transform.scale.y * 0.5f <=
        l->transform.position.y + _entity->transform.scale.y * 0.5)) {
            _entity->velocity.x *= -1.0f;
            _entity->color = InitVector4(1.0f, 0.0f, 0.0f, 1.0f);
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
    ShaderSetMatrix4(_entity->shaderId, "TRANSFORM", transform);
    DrawModel(*_entity->model);

    UnBindShader();
}

void BallOnDestroy(AppContext* _app, Entity* _entity) {

}

void fauxBallUpdate(AppContext* _app, Entity* _entity) {
    Vector3 delta = Vec2ToVec3(Vec2Mul(_entity->velocity, _app->deltaTime));
    _entity->transform.position = Vec3Add(_entity->transform.position, delta);
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