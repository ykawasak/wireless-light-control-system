#ifndef MODEL_HPP
#define MODEL_HPP
#include <stdint.h>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    void sendLoraCommand(uint8_t data);
    
protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
