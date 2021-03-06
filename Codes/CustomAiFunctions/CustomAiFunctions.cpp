//
// Created by dareb on 7/30/2020.
//

#include <Brawl/aiAct.h>
#include <Brawl/ftManager.h>
#include <Wii/MATH.h>
#include <Containers/Vec3f.h>
#include <Containers/vector.h>
#include <Brawl/aiMgr.h>
#include <Graphics/Drawable.h>
#include "Assembly.h"
#include "Brawl/Fighter.h"
#include "Brawl/aiStat.h"

#define OSReport ((void (*)(const char* text, ...)) 0x801d8600)

INJECTION("CUSTOM_AI_FUNCTIONS", 0x8091e104, R"(
    SAVE_REGS
    mr r4, r31
    mr r5, r30
    mr r6, r1
    mr r7, r2
    fmr f1, f10
    bl aiFunctionHandlers
    RESTORE_REGS
    bl outputAiFunctionResult
    cmpwi r12, 0
    beq _CUSTOM_AI_FUNCTIONS_CONTINUE
    lis r12, 0x8091
    ori r12, r12, 0xE440
    mtctr r12
    bctr

_CUSTOM_AI_FUNCTIONS_CONTINUE:
    subi r0,r31,0x1000
)" );

int fn_shouldReturnResult = 0;
extern double md_customFnInjection;

extern "C" {
    double fn_result = 0;
    void aiFunctionHandlers(float unk_f10, aiStat* targetAiStat, unsigned int switchCase, ftInput * selfAi, u32 sp, u32 rtoc) {
        fn_shouldReturnResult = 0;

        if (((switchCase - 0x1000) & 0xFF) == 0xFF) {
            fn_result = md_customFnInjection;
            fn_shouldReturnResult = 1;
            return;
        }

        soModuleAccessor * targetModules;

        // beginning
        bool shouldGetAiTarget = (switchCase & 0x0100) >> 8;
        // get_ref_loop
        while (((u32*)sp)[1] != 0x80764E9C) {
            // get_ref
            sp = *(u32*)sp;
        }
        // get_ref (out of loop)
        targetModules = (*(Fighter**)(sp + 0x20))->modules;
        u32 unk_off3 = *(u32*)(selfAi + 0x74);
        if (shouldGetAiTarget) {
            // get_ref_start
            switchCase -= 0x100;
            // get_target_ref
            int aiInputPtr = *(int*)&targetAiStat->input;
            if (aiInputPtr == 0) {
                return;
            }
            aiInputPtr += 0x140;

            u32 unk_res1_2 = targetAiStat->unkValue;
            if (unk_res1_2 == 0x10) {
                aiInputPtr += 0x280;
            }

            // loc_0x9C
            int entryManagerAddr = *(int*)FIGHTER_MANAGER->entryManager;
            // loc_0xB0
            while (*(int*)(entryManagerAddr + 0x2C) != aiInputPtr) {
                // loc_0xB4
                entryManagerAddr += 0x244;
            }
            if (switchCase - 0x2F == 0x10) {
                entryManagerAddr += 0x08;
            }
            // loc_0xCC
            targetModules = (*(Fighter**)(entryManagerAddr + 0x34))->modules;
        }


        switchCase -= 0x1000;
        if (switchCase == 0x4B) {
            Vec3f targetPos {
                targetModules->groundModule->unk1->unk1->unk1->landingCollisionBottomXPos,
                targetModules->groundModule->unk1->unk1->unk1->landingCollisionBottomYPos,
                targetModules->postureModule->zPos
            };

            targetModules->groundModule->getDistanceFromUnderGrCol(
                    150,
                    &targetPos,
                    true
            );
            asm("stfd f1, 0x00(%0)"
                :
                : "r" (&fn_result));
            fn_shouldReturnResult = 1;
        }
    };
    void outputAiFunctionResult() {
        asm(R"(
            lfd f1, 0x00(%0)
            mr 12, %1
        )"
        :
        : "r" (&fn_result), "r" (fn_shouldReturnResult));
    };
}

INJECTION("CUSTOM_AI_COMMANDS", 0x80917450, R"(
    lbz r4, 0x00(r30)
    SAVE_REGS
    mr r3, r26
    mr r4, r30
    bl aiCommandHandlers
    RESTORE_REGS
)");

#define _get_script_value_aiAct ((double (*)(aiAct * self, int soughtValue, int isPartOfVector)) 0x8091dfc4)
extern vector<Point> pointsToDraw;
extern vector<Line> linesToDraw;
extern vector<RectOutline> rectOutlinesToDraw;
extern "C" {
    void aiCommandHandlers(aiAct* aiActInst, const int* args) {
        int cmd = (args[0] & 0xFF000000) >> 24;
        if (cmd < 0x35) return;
        if (cmd <= 0x3A) {
            int varToMod = args[1];
            double index = _get_script_value_aiAct(aiActInst, *(int *) &args[2], 0);
            bool shouldGetTarget = _get_script_value_aiAct(aiActInst, *(int *) &args[3], 0);

            soWorkManageModuleImpl* targetWorkModule;
            if (shouldGetTarget) {
                auto targetPlayerNum = AI_MANAGER->getAiCpuTarget(FIGHTER_MANAGER->getPlayerNo(aiActInst->ftInputPtr->fighterId));
                Fighter* target = FIGHTER_MANAGER->getFighter(FIGHTER_MANAGER->getEntryId(targetPlayerNum));
                if (target == nullptr) return;
                targetWorkModule = target->modules->workModule;
            } else {
                targetWorkModule = FIGHTER_MANAGER->getFighter(aiActInst->ftInputPtr->fighterId)->modules->workModule;
            }

            switch (cmd) {
                case 0x35:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->RAVariables->bitsSize]) (targetWorkModule->RAVariables->bitVariables))[(int) index];
                    return;
                case 0x36:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->RAVariables->basicsSize]) (targetWorkModule->RAVariables->basics))[(int) index];
                    return;
                case 0x37:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->RAVariables->floatsSize]) (targetWorkModule->RAVariables->floats))[(int) index];
                    return;
                case 0x38:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->LAVariables->bitsSize]) (targetWorkModule->LAVariables->bitVariables))[(int) index];
                    return;
                case 0x39:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->LAVariables->basicsSize]) (targetWorkModule->LAVariables->basics))[(int) index];
                    return;
                case 0x3A:
                    aiActInst->variables[varToMod] = (float) (*(int (*)[targetWorkModule->LAVariables->floatsSize]) (targetWorkModule->LAVariables->floats))[(int) index];
                    return;
            }
        }
        if (cmd < 0x50) {
            switch (cmd) {
                case 0x40: {
                    double x1 = _get_script_value_aiAct(aiActInst, *(int *) &args[1], 0);
                    double y1 = _get_script_value_aiAct(aiActInst, *(int *) &args[2], 0);
                    int red = _get_script_value_aiAct(aiActInst, *(int *) &args[3], 0);
                    int green = _get_script_value_aiAct(aiActInst, *(int *) &args[4], 0);
                    int blue = _get_script_value_aiAct(aiActInst, *(int *) &args[5], 0);
                    int alpha = _get_script_value_aiAct(aiActInst, *(int *) &args[6], 0);
                    pointsToDraw.push({
                              0x000000FF,
                              (float) x1,
                              (float) y1,
                              42
                      });
                    pointsToDraw.push({
                              (red << 24) | (green << 16) | (blue << 8) | alpha,
                              (float) x1,
                              (float) y1,
                              30
                      });
                    return;
                }
                case 0x41: {
                    double x1 = _get_script_value_aiAct(aiActInst, *(int *) &args[1], 0);
                    double y1 = _get_script_value_aiAct(aiActInst, *(int *) &args[2], 0);
                    double x2 = _get_script_value_aiAct(aiActInst, *(int *) &args[3], 0);
                    double y2 = _get_script_value_aiAct(aiActInst, *(int *) &args[4], 0);
                    int red = _get_script_value_aiAct(aiActInst, *(int *) &args[5], 0);
                    int green = _get_script_value_aiAct(aiActInst, *(int *) &args[6], 0);
                    int blue = _get_script_value_aiAct(aiActInst, *(int *) &args[7], 0);
                    int alpha = _get_script_value_aiAct(aiActInst, *(int *) &args[8], 0);
                    linesToDraw.push({
                             0x000000FF,
                             (float) x1,
                             (float) y1,
                             (float) x2,
                             (float) y2,
                             42
                     });
                    linesToDraw.push({
                             (red << 24) | (green << 16) | (blue << 8) | alpha,
                             (float) x1,
                             (float) y1,
                             (float) x2,
                             (float) y2,
                             30
                     });
                    return;
                }
                case 0x42: {
                    double x = _get_script_value_aiAct(aiActInst, *(int *) &args[1], 0);
                    double y = _get_script_value_aiAct(aiActInst, *(int *) &args[2], 0);
                    double width = _get_script_value_aiAct(aiActInst, *(int *) &args[3], 0);
                    double height = _get_script_value_aiAct(aiActInst, *(int *) &args[4], 0);
                    int red = _get_script_value_aiAct(aiActInst, *(int *) &args[5], 0);
                    int green = _get_script_value_aiAct(aiActInst, *(int *) &args[6], 0);
                    int blue = _get_script_value_aiAct(aiActInst, *(int *) &args[7], 0);
                    int alpha = _get_script_value_aiAct(aiActInst, *(int *) &args[8], 0);
                    rectOutlinesToDraw.push({
                            0x000000FF,
                            (float) (y - height),
                            (float) (y + height),
                            (float) (x - width),
                            (float) (x + width),
                            42
                    });
                    rectOutlinesToDraw.push({
                        (red << 24) | (green << 16) | (blue << 8) | alpha,
                        (float) (y - height),
                        (float) (y + height),
                        (float) (x - width),
                        (float) (x + width),
                        30
                    });
                    return;
                }
            }
        }
        if (cmd < 0x60) {
            auto colorModule = FIGHTER_MANAGER->getFighter(aiActInst->ftInputPtr->fighterId)->modules->colorBlendModule;
            switch (cmd) {
                case 0x50:
                    colorModule->isEnabled = 1;
                    return;
                case 0x51:
                    colorModule->isEnabled = 0;
                    return;
                case 0x52:
                    int red = _get_script_value_aiAct(aiActInst, *(int *) &args[1], 0);
                    int green = _get_script_value_aiAct(aiActInst, *(int *) &args[2], 0);
                    int blue = _get_script_value_aiAct(aiActInst, *(int *) &args[3], 0);
                    int alpha = _get_script_value_aiAct(aiActInst, *(int *) &args[4], 0);
                    colorModule->red = red;
                    colorModule->green = green;
                    colorModule->blue = blue;
                    colorModule->alpha = alpha;
                    return;
            }
        }
        if (cmd < 0x70) {
            switch (cmd) {
                case 0x60:
                    OSReport("LOGGED VALUE: %.3f\n", _get_script_value_aiAct(aiActInst, *(int *) &args[1], 0));
                    return;
                case 0x61:
                    OSReport("LOGGED STRING: ");
                    for (int i = 1; i <= 5; i++) {
                        unsigned int toConvert = _get_script_value_aiAct(aiActInst, *(int *) &args[i], 0);
                        OSReport("%c%c%c",
                                (toConvert & 0xFF000000) >> 24,
                                (toConvert & 0x00FF0000) >> 16,
                                (toConvert & 0x0000FF00) >> 8);
                    }
                    OSReport("\n");
            }
        }
    }
}