#include "utils.h"

void print(const glm::vec4& v, const QString& name)
{
    qDebug() << name << ": " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
}

void print(const glm::vec3& v, const QString& name)
{
    qDebug() << name << ": " << v[0] << ", " << v[1] << ", " << v[2];
}

void print(const float f, const QString& name)
{
    qDebug() << name << ": " << f;
}

void printState(const int i)
{
    switch(i)
    {
    case 0:
        qDebug() << "Walking!";
        break;
    case 1:
        qDebug() << "Flying!";
        break;
    case 2:
        qDebug() << "Weightlessness!";
        break;
    case 3:
        qDebug() << "Swimming!";
        break;
    default:
        break;
    }
}

void printBlock(const int i)
{
    switch(i)
    {
    case 0:
        qDebug() << "EMPTY!";
        break;
    case 1:
        qDebug() << "GRASS!";
        break;
    case 2:
        qDebug() << "DIRT!";
        break;
    case 3:
        qDebug() << "STONE!";
        break;
    case 4:
        qDebug() <<  "STONE!";
        break;
    case 5:
        qDebug() << "LAVA!";
        break;
    case 6:
        qDebug() << "WATER!";
        break;
    default:
        break;
    }
}
