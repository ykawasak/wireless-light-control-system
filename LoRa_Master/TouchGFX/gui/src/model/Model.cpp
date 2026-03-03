#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "cmsis_os.h"

extern osMessageQueueId_t loraQueueHandle;

Model::Model() : modelListener(0)
{

}

void Model::tick()
{

}

void Model::sendLoraCommand(uint8_t data) 
{
    osMessageQueuePut(loraQueueHandle, &data, 0, 0);
}