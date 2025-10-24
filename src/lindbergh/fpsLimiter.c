#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#include "fpsLimiter.h"

FpsLimit fpsLimit;
static struct timespec nextFrameTime;

static inline void clockNow(struct timespec *ts)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, ts);
}

static inline long toMicro(const struct timespec *ts)
{
    return (long)(ts->tv_sec * 1000000L + ts->tv_nsec / 1000L);
}

static inline void addMicro(struct timespec *ts, long micro)
{
    ts->tv_nsec += (micro % 1000000L) * 1000L;
    ts->tv_sec += micro / 1000000L;
    if (ts->tv_nsec >= 1000000000L)
    {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000L;
    }
}

void initFpsLimiter()
{
    if (getConfig()->fpsLimiter == 1)
    {
        double targetFps = getConfig()->fpsTarget;

        fpsLimit.targetFrameTime = (long)(1000000.0 / targetFps + 0.5);

        clockNow(&nextFrameTime);
    }
}

void frameTiming()
{
    struct timespec now;
    clockNow(&now);

    long currentMicro = toMicro(&now);
    long targetMicro = toMicro(&nextFrameTime);

    if (currentMicro < targetMicro)
    {
        long sleepMicro = targetMicro - currentMicro;
        if (sleepMicro > 1000)
        {
            struct timespec sleepTime;
            sleepTime.tv_sec = sleepMicro / 1000000L;
            sleepTime.tv_nsec = (sleepMicro % 1000000L) * 1000L;

            int ret = clock_nanosleep(CLOCK_MONOTONIC_RAW, 0, &sleepTime, NULL);
            (void)ret;
        }
        else if (sleepMicro > 100)
        {
            usleep(sleepMicro - 50);
        }
        do
        {
            clockNow(&now);
            currentMicro = toMicro(&now);
        } while (currentMicro < targetMicro);
    }

    addMicro(&nextFrameTime, fpsLimit.targetFrameTime);

    clockNow(&now);
    long diffMicro = toMicro(&now) - toMicro(&nextFrameTime);
    if (diffMicro > fpsLimit.targetFrameTime * 2)
    {
        nextFrameTime = now;
        addMicro(&nextFrameTime, fpsLimit.targetFrameTime);
    }
}

void fpsLimiter(FpsLimit *stats)
{
    (void)stats;
    frameTiming();
}

double calculateFps()
{
    static double lastTime = 0.0;
    static int frameCount = 0;
    static double fps = 0.0;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    double currentTime = ts.tv_sec + ts.tv_nsec / 1e9;
    double delta = currentTime - lastTime;
    frameCount++;

    if (delta >= 1.0)
    {
        fps = frameCount / delta;
        frameCount = 0;
        lastTime = currentTime;
    }

    return fps;
}