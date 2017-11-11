#pragma once

abstract_class IMovementListener
{
public:
    virtual ~IMovementListener() {};

    virtual void OnPlayerLand() = 0;
    virtual void OnPlayerJump() = 0;
};