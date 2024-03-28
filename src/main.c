#include <graphx.h>
#include <math.h>
#include <string.h>
#include <keypadc.h>
#include <sys/util.h>
#include <sys/rtc.h>
#include <fileioc.h>
#include "./gfx/gfx.h"
// handy: subtract 10 from line number to find actual block id! 
gfx_sprite_t* blocks[] = {
    0,
    acacia_log,
    acacia_planks,
    bedrock,
    birch_log,
    birch_planks,
    bookshelf,
    bricks,
    cactus_side,
    clay,
    coal_ore, //ore
    coarse_dirt,
    cobblestone,
    copper_block,
    copper_ore, //ore
    crafting_table_front,
    cut_copper,
    dark_oak_log,
    dark_oak_planks,
    deepslate,
    diamond_ore, //ore
    diorite,
    dirt_path_side,
    dirt,
    emerald_ore, //ore
    enchanting_table_side,
    farmland,
    farmland_moist,
    furnace_front,
    furnace_front_on,
    glass,
    gold_ore, //ore
    granite,
    grass_block_side,
    grass_block_snow,
    gravel,
    hay_block_side,
    iron_ore, //ore
    jungle_log,
    jungle_planks,
    ladder,
    lapis_ore, //ore
    mangrove_log,
    mangrove_planks,
    mossy_cobblestone,
    oak_door_top,
    oak_door_bottom,
    oak_log,
    oak_planks,
    obsidian,
    podzol_side,
    pumpkin_side,
    redstone_ore, //ore
    sand,
    sandstone,
    smooth_stone,
    snow,
    soul_sand,
    spruce_log,
    spruce_planks,
    stone,
    water,
    oak_leaves, // begin block update 1
    barrel_side,
    blast_furnace_front,
    cake_side,
    cobbled_deepslate,
    deepslate_tiles,
    deepslate_coal_ore,
    deepslate_copper_ore,
    deepslate_diamond_ore,
    deepslate_emerald_ore,
    deepslate_gold_ore,
    deepslate_iron_ore,
    deepslate_lapis_ore,
    deepslate_redstone_ore,
    diamond_block,
    emerald_block,
    gold_block,
    glowstone,
    iron_block,
    honeycomb_block,
    mud_bricks,
    mossy_stone_bricks,
    mycelium_side,
    oak_sapling,
    orange_tulip,
    polished_blackstone,
    polished_deepslate,
    polished_blackstone_bricks,
    reinforced_deepslate_side,
    smithing_table_front,
    stone_bricks,
    sugar_cane,
    tnt_side,
    torch,
    charcoal, // begin items
    coal,
    cookie,
    copper_ingot,
    diamond,
    emerald,
    flint_and_steel,
    flint,
    gold_ingot,
    iron_ingot,
    raw_copper,
    raw_gold,
    raw_iron,
    stone_axe,
    stone_hoe,
    stone_pickaxe,
    stone_shovel,
    stone_sword,
    wheat_seeds,
    wooden_axe,
    wooden_hoe,
    wooden_pickaxe,
    wooden_shovel,
    wooden_sword,
    map
};
uint8_t transparentblocks[] = {
    8,22,25,30,40,45,46,62,65,85,86,93,95
};
uint8_t nonblockingblocks[] = {
    0,40,85,86,93,95
};
// handy: subtract 10 from line number to find actual block id! 
// game related stuff

// if !creativemode then survivalmode
bool creativemode = true;
int elevation = 64;
int elevationchange = 0;
int cavey = 0;
bool step = false;
bool isbeach = false;
bool onground = false;
uint8_t blockidground = 0;
uint8_t blockidleft = 0;
uint8_t blockidright = 0;
uint8_t blockidceil = 0;
bool hitceil = false;
bool blockleft = false;
bool blockright = false;
bool backtile = false;
bool sndkeypress = false;
bool alphakeypress = false;
bool modekeypress = false;
uint8_t thisblocka = false;
uint8_t thisblockb = false;
uint8_t inblock = 0;
float playerx = 0;
float playery = 0;
float xoffset = 0;
float yoffset = 0;
float playerxvel = 0;
float playeryvel = 0;
bool playerleft = false;
bool didloop = false;
bool onladder = false;
uint8_t crafting[3][3];
uint8_t craftingoutput = 0;
uint8_t worlda[220][100];
uint8_t worldb[220][100];
int worldwidth = 219;
int worlddepth = 99;
int cursorx;
int cursory;
uint8_t prevblock = 0;
void* freeram = 0;
/*
cursorh do be weird. Subtract two from its value for the height relative to the player. Maybe.
    454
    3#3
    2#2
    101
*/
int cursorh = 2;
int scrollfactor = 0;
uint8_t scrolly = 0;
uint8_t elevations[220];
bool trees[220];
uint8_t numsteps = 0;
// gui related stuff
uint8_t currentslot = 0;
uint8_t hotbar[5] = {0};
uint8_t inventorydata[27] = {0};
uint8_t armorslots[4] = {0};
uint8_t hotbarquant[5] = {0};
uint8_t inventoryquant[27] = {0};
uint8_t selectdata = 0;
uint8_t selectquant = 0;
bool selectedblock = false;
bool inventoryopen = false;
uint8_t titleselect = 1;
bool keypressed = false;
char* titlebuttonnames[] = {
    "Continue World",
    "Create New World",
    "Quit Game"
};
uint8_t worldselect = 0;
uint8_t invcursorx = 0;
uint8_t invcursory = 4;
uint8_t thistile = 0;
bool istransparent(uint8_t blockid) {
    bool istrans = false;
    for (int i = 0; i < 13; i++) {
        if (transparentblocks[i] == blockid || blockid >= 96) {
            istrans = true;
        }
    }
    return istrans;
}
bool isnonblocking(uint8_t blockid) {
    bool isnonblock = false;
    for (int i = 0; i < 6; i++) {
        if (nonblockingblocks[i] == blockid || blockid >= 96) {
            isnonblock = true;
        }
    }
    return isnonblock;
}
int setboth(uint8_t x, uint8_t y, uint8_t blockid) {
    worlda[x][y] = blockid;
    worldb[x][y] = blockid;
    return 0;
}

#define nopano

int main(void) {
    // set up runtime data
    ti_SetGCBehavior(gfx_End,gfx_Begin);
    srand(rtc_Time());
    uint8_t splashnum = randInt(0,46);
    gfx_sprite_t* blockscaled = gfx_MallocSprite(32,32);
    // gfx_sprite_t* mapscaled = gfx_MallocSprite(256,256);
    gfx_sprite_t* steveright = gfx_MallocSprite(16,64);
    gfx_sprite_t* alexright = gfx_MallocSprite(16,64);
    gfx_FlipSpriteY(steve,steveright);
    gfx_FlipSpriteY(alex,alexright);
    // gfx_ScaleSprite(map_background,mapscaled);
    char* splashtext[] = {
        "Revolutionary!",
        "Now on a calculator near you!",
        "100 percent pure!",
        "Bigger than a breadbox!",
        "Less polygons!",
        "As seen on TV!",
        "Limited edition!",
        "Flashing letters!",
        "Kind of dragon free!",
        "Ingots!",
        "Exploding creepers!",
        "That's no moon!",
        "Hans shot first!",
        "90 percent bug free!",
        "12 herbs and spices!",
        "Technically good!",
        "150 percent hyperbole!",
        "Don't look directly at the bugs!",
        "Lots of truthiness!",
        "uint8_t splashnum = 19;!",
        "missingno",
        "Indev!",
        "Spiders everywhere!",
        "Check it out!",
        "Holy cow, man!",
        "It's a game!",
        "Made in Texas!",
        "Uses graphx.h!",
        "Reticulating splines!",
        "Minecraft!",
        "Yaaay!",
        "Singleplayer!",
        "Keypad compatible!",
        "l33t!",
        "Create!",
        "Survive!",
        "Dungeon!",
        "Exclusive!",
        "The bee's knees!",
        "Open source!",
        "Classy!",
        "Wow!",
        "Not on Steam!",
        "Oh man!",
        "RAM Cleared!",
        "It's here!",
        "Synecdoche!"
    };
    // for(uint8_t x = 0; x < 50; x++) {
    //     for(uint8_t y = 0; y < 55; y++) {
    //         if (x+y < 60) {
    //             worlda[x][y] = x+y;
    //         }
    //     }
    // }
    // make drawrings
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(mcpal,sizeof_mcpal,0);
    gfx_FillScreen(4);
    gfx_SetTextFGColor(5);
    gfx_SetTransparentColor(0);
    gfx_SetTextTransparentColor(255);
    #ifdef nopano
        for(uint8_t x = 0; x < 20; x++) {
            for(uint8_t y = 0; y < 15; y++) {
                gfx_Sprite(dirt,x*16,y*16);
            }
        }
        gfx_Sprite(logo,48,48);
    #else
        gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
        gfx_TransparentSprite(logo_transparent,48,48);
    #endif
    gfx_SetTextFGColor(1);
    gfx_PrintStringXY(splashtext[splashnum],301-gfx_GetStringWidth(splashtext[splashnum]),96);
    gfx_SetTextFGColor(5);
    gfx_PrintStringXY(splashtext[splashnum],300-gfx_GetStringWidth(splashtext[splashnum]),95);
    gfx_SetTextFGColor(3);
    gfx_PrintStringXY("v0.10.0",160-gfx_GetStringWidth("v0.10.0")/2,220);
    gfx_PrintStringXY("RAM Free: ",5,220);
    gfx_PrintUInt(os_MemChk(&freeram),0);
    gfx_SwapDraw();
    gfx_SetDrawScreen();
    do {
        for (uint8_t i = 1; i < 3; i++) {
            gfx_SetColor(2);
            gfx_FillRectangle_NoClip(60,120+(i*25),200,20);
            if (titleselect == i) {
                gfx_SetColor(3);
                gfx_Rectangle_NoClip(60,120+(i*25),200,20);
            } else {
                gfx_SetColor(1);
                gfx_Rectangle_NoClip(60,120+(i*25),200,20);
            }
            gfx_PrintStringXY(titlebuttonnames[i],160-(gfx_GetStringWidth(titlebuttonnames[i])/2),126+(i*25));
        }
        while (kb_AnyKey()) {kb_Scan();}
        while (!kb_AnyKey()) {kb_Scan();}
        if ((kb_Data[7] & kb_Up) && titleselect > 1) {
            titleselect -= 1;
            // keypressed = true;
        }
        if ((kb_Data[7] & kb_Down) && titleselect < 2) {
            titleselect += 1;
            // keypressed = true;
        }
    } while(!(kb_Data[1] & kb_2nd));
    if (titleselect == 3) {
        gfx_End();
        return 0;
    } else if (titleselect == 1) {
        gfx_SetDrawBuffer();
        // #ifdef nopano
        //     for(uint8_t x = 0; x < 20; x++) {
        //         for(uint8_t y = 0; y < 15; y++) {
        //             gfx_Sprite(dirt,x*16,y*16);
        //         }
        //     }
        //     gfx_Sprite(logo,48,48);
        // #else
        //     gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
        //     gfx_TransparentSprite(logo_transparent,48,48);
        // #endif
        // gfx_SetColor(2);
        // gfx_FillRectangle(85,20,150,200);
        // gfx_SetColor(1);
        // gfx_Rectangle(90,35,140,170);
        // gfx_SetColor(3);
        // gfx_Rectangle(92,20,135,20);
        // gfx_SetTextFGColor(1);
        // gfx_PrintStringXY("Select world...",90,24);
        // for(uint8_t i = 0; i < 10; i++) {
        //     gfx_PrintStringXY("Select world...",90,24);
        //     gfx_Rectangle(92,20,135,20);
        // }
        // gfx_SetColor(0);
        // gfx_SwapDraw();
        // gfx_SetTextFGColor(3);
        // while (kb_Data[1] & kb_2nd) {kb_Scan();}
        // do {
        //     kb_Scan();
        //     if (kb_Data[7] & kb_Up) {
        //         worldselect++;
        //     }
        //     if (kb_Data[7] & kb_Down) {
        //         worldselect--;
        //     }
        // } while(!(kb_Data[1] & kb_2nd));
        #ifdef nopano
            for(uint8_t x = 0; x < 20; x++) {
                for(uint8_t y = 0; y < 15; y++) {
                    gfx_Sprite(dirt,x*16,y*16);
                }
            }
            gfx_Sprite(logo,48,48);
        #else
            gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
            gfx_TransparentSprite(logo_transparent,48,48);
        #endif
        gfx_PrintStringXY("Generating Terrain",160-(gfx_GetStringWidth("Generating Terrain")/2),150);
        gfx_SwapDraw();
        // not-so-basic world generation ________________________________________________________________________________
        // make surface height
        for(uint8_t x = 0; x < worldwidth; x++) {
            step = randInt(0,1);
            if (step) {
                elevationchange = 0;
                numsteps++;
            } else {
                elevationchange += randInt(-1,1);
                elevation += elevationchange;
                if (elevation > 100) elevation = 100;
                if (elevation < 30) elevation = 30;
            }
            if (elevationchange > 2 || elevationchange < -2) {
                elevationchange = 0;
            }
            elevations[x] = elevation;
            trees[x] = false;
            if (numsteps > 9) {
                numsteps = 0;
                trees[x] = true;
            }
        }
        #ifdef nopano
            for(uint8_t x = 0; x < 20; x++) {
                for(uint8_t y = 0; y < 15; y++) {
                    gfx_Sprite(dirt,x*16,y*16);
                }
            }
            gfx_Sprite(logo,48,48);
        #else
            gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
            gfx_TransparentSprite(logo_transparent,48,48);
        #endif
        gfx_PrintStringXY("Generating Blocks",160-(gfx_GetStringWidth("Generating Blocks")/2),150);
        gfx_SwapDraw();
        // actually generate the blocks for provided surface height
        for(int x = 0; x < worldwidth; x++) {
            for(int y = elevations[x]; y < worlddepth; y++) {
                if (y < 75) {
                    isbeach = false;
                } else {
                    isbeach = true;
                }
                if (y - elevations[x] == 0) {
                    if (isbeach) {
                        worlda[x][y] = 53;
                    } else {
                        worlda[x][y] = 33;
                    }
                } else if ((y - elevations[x] > 0) && (y - elevations[x] < (int)randInt(3,7))) {
                    if (isbeach) {
                        worlda[x][y] = 53;
                    } else {
                        worlda[x][y] = 23;
                    }
                } else if (y - elevations[x] > 0) {
                    // if (y == 127) {
                    //     worlda[x][y] = 3;
                    // } else {
                    //     worlda[x][y] = 60;
                    // }
                    uint8_t isore = randInt(0,30);
                    if (isore > 22) {
                        if (isore == 23) worlda[x][y] = 10;
                        if (isore == 24) worlda[x][y] = 14;
                        if (isore == 25) worlda[x][y] = 20;
                        if (isore == 26) worlda[x][y] = 24;
                        if (isore == 27) worlda[x][y] = 31;
                        if (isore == 28) worlda[x][y] = 37;
                        if (isore == 29) worlda[x][y] = 41;
                        if (isore == 30) worlda[x][y] = 52;
                    } else {
                        worlda[x][y] = 60;
                    }
                } else {
                    worlda[x][y] = 0;
                }
            }
        }
        #ifdef nopano
            for(uint8_t x = 0; x < 20; x++) {
                for(uint8_t y = 0; y < 15; y++) {
                    gfx_Sprite(dirt,x*16,y*16);
                }
            }
            gfx_Sprite(logo,48,48);
        #else
            gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
            gfx_TransparentSprite(logo_transparent,48,48);
        #endif
        gfx_PrintStringXY("Almost ready...",160-(gfx_GetStringWidth("Almost ready...")/2),150);
        gfx_SwapDraw();
        // copy world to second array
        for(int i = 0; i < worldwidth; i++) {
            for(int i2 = 0; i2 < worlddepth; i2++) {
                worldb[i][i2] = worlda[i][i2];
            }
        }
        // special features (trees, caves)
        for(uint8_t x = 0; x < worldwidth; x++) {
            if (trees[x]) {
                if (elevations[x] < 75) {
                    worldb[x][elevations[x]-1] = 47;
                    worldb[x][elevations[x]-2] = 47;
                    worldb[x][elevations[x]-3] = 47;
                    worldb[x][elevations[x]-4] = 47;
                    worldb[x][elevations[x]-5] = 47;
                    worlda[x][elevations[x]-4] = 62;
                    setboth(x-1,elevations[x]-4,62);
                    setboth(x-2,elevations[x]-4,62);
                    setboth(x+1,elevations[x]-4,62);
                    setboth(x+2,elevations[x]-4,62);
                    worlda[x][elevations[x]-5] = 62;
                    setboth(x-1,elevations[x]-5,62);
                    setboth(x-2,elevations[x]-5,62);
                    setboth(x+1,elevations[x]-5,62);
                    setboth(x+2,elevations[x]-5,62);
                    setboth(x,elevations[x]-6,62);
                    setboth(x,elevations[x]-7,62);
                    worldb[x-1][elevations[x]-6] = 62;
                    worldb[x-1][elevations[x]-7] = 62;
                    worldb[x+1][elevations[x]-6] = 62;
                    worldb[x+1][elevations[x]-7] = 62;
                } else {
                    worldb[x][elevations[x]-1] = 8;
                    worldb[x][elevations[x]-2] = 8;
                }
            }
        }
        playerx = 75;
        playery = elevations[75]-1;
        cursory = playery + (cursorh - 2);
        if (cursorh == 1 || cursorh == 2) {
            cursorx = playerx - (2*playerleft);
        } else {
            cursorx = playerx;
        }
    } else if (titleselect == 0) {
        #ifdef nopano
            for(uint8_t x = 0; x < 20; x++) {
                for(uint8_t y = 0; y < 15; y++) {
                    gfx_Sprite(dirt,x*16,y*16);
                }
            }
            gfx_Sprite(logo,48,48);
        #else
            gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
            gfx_TransparentSprite(logo_transparent,48,48);
        #endif
        // gfx_PrintStringXY("Loading world",160-(gfx_GetStringWidth("Loading world")/2),150);
        // gfx_SwapDraw();
        // uint8_t importdata = ti_Open("MCEWORLD","r");
        // void* readdata = 0;
        // if (importdata != 0) {
        //     ti_Rewind(importdata);
        //     for (int y = 0; y < worlddepth; y++) {
        //         for (int x = 0; x < worldwidth; x++) {
        //             readdata = &worlda[x][y];
        //             ti_Read(readdata,1,1,importdata);
        //         }
        //     }
        //     for (int y = 0; y < worlddepth; y++) {
        //         for (int x = 0; x < worldwidth; x++) {
        //             readdata = &worldb[x][y];
        //             ti_Read(readdata,1,1,importdata);
        //         }
        //     }
        //     uint8_t playerxsave = 0;
        //     uint8_t playerysave = 0;
        //     readdata = &playerxsave;
        //     ti_Read(readdata,1,1,importdata);
        //     readdata = &playerysave;
        //     ti_Read(readdata,1,1,importdata);
        //     playery = playerysave;
        //     playerx = playerxsave;
        // }
        // ti_Close(importdata);
    }
    if (titleselect == 0 || titleselect == 1) {    
        do {
            gfx_FillScreen(4);
            if (inventoryopen || hotbar[currentslot] == 120) {gfx_FillScreen(7);}
            // deprecated 1x renderer - possibly implement later as optional
            // for(int x = 0; x < 20; x++) {
            //     for(int y = 0; y < 15; y++) {
            //         backtile = false;
            //         if (worldb[(int)(x+playerx-10)][(int)(y+playery-8)] != 0) {
            //             gfx_Sprite(blocks[worldb[(int)(x+playerx-10)][(int)(y+playery-8)]],(x+(playerx)-roundf(playerx))*16,(y+(playery)-roundf(playery))*16);
            //             backtile = true;
            //         }
            //         if (worlda[(int)(x+playerx-10)][(int)(y+playery-8)] != 0) {
            //             gfx_Sprite(blocks[worlda[(int)(x+playerx-10)][(int)(y+playery-8)]],(x+(playerx)-roundf(playerx))*16,(y+(playery)-roundf(playery))*16);
            //         } else if (backtile) {
            //             gfx_TransparentSprite(shading,(x+(playerx)-roundf(playerx))*16,(y+(playery)-roundf(playery))*16);
            //         }
            //     }
            // }

            // 2x renderer + optimizations
            gfx_ScaledSprite_NoClip(sun,110-(playerx/3),30,2,2);
            if (inventoryopen || hotbar[currentslot] == 120) {
                for(int x = 0; x < 10; x++) {
                    for(int y = 0; y < 8; y++) {
                        if ((worlda[(int)(x+playerx-5)][(int)(y+playery-4)] != 0) || (worldb[(int)(x+playerx-5)][(int)(y+playery-4)] != 0)) {
                            gfx_FillRectangle(x*32,y*32,32,32);
                        }       
                    }
                }
            } else {
                xoffset = fabsf(playerx - ceilf(playerx));
                yoffset = fabsf(playery - ceilf(playery));
                for(float x = xoffset-1; x < 10+xoffset; x++) {
                    for(float y = yoffset-1; y < 8+yoffset; y++) {
                        thisblocka = worlda[(int)(x+playerx-5)][(int)(y+playery-4)];
                        thisblockb = worldb[(int)(x+playerx-5)][(int)(y+playery-4)];
                        if (x > 0 && x < 9 && y > 0 && y < 6) {
                            if (thisblocka != 0) {
                                if (istransparent(thisblocka)) {
                                    if (thisblockb != 0) {
                                        if (prevblock != thisblockb) {
                                            gfx_ScaleSprite(blocks[thisblockb],blockscaled);
                                            prevblock = thisblockb;
                                        }
                                        if (istransparent(thisblockb)) {
                                            gfx_TransparentSprite_NoClip(blockscaled,x*32,y*32);
                                        } else {
                                            gfx_Sprite_NoClip(blockscaled,x*32,y*32);
                                        }
                                        gfx_TransparentSprite_NoClip(shading,x*32,y*32);
                                        gfx_TransparentSprite_NoClip(shading,x*32+16,y*32);
                                        gfx_TransparentSprite_NoClip(shading,x*32+16,y*32+16);
                                        gfx_TransparentSprite_NoClip(shading,x*32,y*32+16);
                                    }
                                    if (prevblock != thisblocka) {
                                        gfx_ScaleSprite(blocks[thisblocka],blockscaled);
                                        prevblock = thisblocka;
                                    }
                                    gfx_TransparentSprite_NoClip(blockscaled,x*32,y*32);
                                } else {
                                    if (prevblock != thisblocka) {
                                        gfx_ScaleSprite(blocks[thisblocka],blockscaled);
                                        prevblock = thisblocka;
                                    }
                                    gfx_Sprite_NoClip(blockscaled,x*32,y*32);
                                }
                            } else {
                                if (thisblockb != 0) {
                                    if (prevblock != thisblockb) {
                                        gfx_ScaleSprite(blocks[thisblockb],blockscaled);
                                        prevblock = thisblockb;
                                    }
                                    if (istransparent(thisblockb)) {
                                        gfx_TransparentSprite_NoClip(blockscaled,x*32,y*32);
                                    } else {
                                        gfx_Sprite_NoClip(blockscaled,x*32,y*32);
                                    }
                                    gfx_TransparentSprite_NoClip(shading,x*32,y*32);
                                    gfx_TransparentSprite_NoClip(shading,x*32+16,y*32);
                                    gfx_TransparentSprite_NoClip(shading,x*32+16,y*32+16);
                                    gfx_TransparentSprite_NoClip(shading,x*32,y*32+16);
                                }
                            } 
                        } else {
                            if (thisblocka != 0) {
                                if (istransparent(thisblocka)) {
                                    if (thisblockb != 0) {
                                        if (prevblock != thisblockb) {
                                            gfx_ScaleSprite(blocks[thisblockb],blockscaled);
                                            prevblock = thisblockb;
                                        }
                                        if (istransparent(thisblockb)) {
                                            gfx_TransparentSprite(blockscaled,x*32,y*32);
                                        } else {
                                            gfx_Sprite(blockscaled,x*32,y*32);
                                        }
                                        gfx_TransparentSprite(shading,x*32,y*32);
                                        gfx_TransparentSprite(shading,x*32+16,y*32);
                                        gfx_TransparentSprite(shading,x*32+16,y*32+16);
                                        gfx_TransparentSprite(shading,x*32,y*32+16);
                                    }
                                    if (prevblock != thisblocka) {
                                        gfx_ScaleSprite(blocks[thisblocka],blockscaled);
                                        prevblock = thisblocka;
                                    }
                                    gfx_TransparentSprite(blockscaled,x*32,y*32);
                                } else {
                                    if (prevblock != thisblocka) {
                                        gfx_ScaleSprite(blocks[thisblocka],blockscaled);
                                        prevblock = thisblocka;
                                    }
                                    gfx_Sprite(blockscaled,x*32,y*32);
                                }
                            } else {
                                if (thisblockb != 0) {
                                    if (prevblock != thisblockb) {
                                        gfx_ScaleSprite(blocks[thisblockb],blockscaled);
                                        prevblock = thisblockb;
                                    }
                                    if (istransparent(thisblockb)) {
                                        gfx_TransparentSprite(blockscaled,x*32,y*32);
                                    } else {
                                        gfx_Sprite(blockscaled,x*32,y*32);
                                    }
                                    gfx_TransparentSprite(shading,x*32,y*32);
                                    gfx_TransparentSprite(shading,x*32+16,y*32);
                                    gfx_TransparentSprite(shading,x*32+16,y*32+16);
                                    gfx_TransparentSprite(shading,x*32,y*32+16);
                                }
                            } 
                        }      
                    }
                }
                gfx_Rectangle_NoClip((cursorx-(playerx-5))*32,(cursory-(playery-3))*32,32,32);
                if (playerleft) {
                    gfx_ScaledTransparentSprite_NoClip(steve,(32*4.8),(32*3),2,2);
                } else {
                    gfx_ScaledTransparentSprite_NoClip(steveright,(32*4.8),(32*3),2,2);
                }
                if (hotbar[currentslot] != 0) {
                    if (istransparent(hotbar[currentslot])) {
                        gfx_TransparentSprite(blocks[hotbar[currentslot]],32*4.8,32*4);
                    } else {
                        gfx_Sprite(blocks[hotbar[currentslot]],32*4.8,32*4);
                    }
                }
                for(int i = 0; i < 5; i++) {
                    gfx_Sprite_NoClip(hotbarslot,110+i*20,210);
                    if (hotbar[i] != 0) {
                        if (istransparent(hotbar[i])) {
                            gfx_TransparentSprite_NoClip(blocks[hotbar[i]],112+i*20,212);
                        } else {
                            gfx_Sprite_NoClip(blocks[hotbar[i]],112+i*20,212);
                        }
                    }
                }
                gfx_TransparentSprite_NoClip(selectslot,109+currentslot*20,209);
                if (!creativemode) {
                    for(int i = 0; i < 5; i++) {
                        if (hotbarquant[i] > 1) {
                            gfx_SetTextXY(72+44+i*20,212+10);
                            gfx_PrintUInt(hotbarquant[i],0);
                        }
                    }
                }
                gfx_SetTextXY(1,1);
                gfx_SetColor(0);
                gfx_FillRectangle_NoClip(0,0,140,10);
                gfx_PrintString("Position: X:");
                gfx_PrintUInt(playerx,0);
                gfx_PrintString(" Y:");
                gfx_PrintUInt(playery,0);
            }
            if (inventoryopen) {
                if (creativemode) {
                    scrollfactor = (100/((int)ceil(121/9)-1));
                    gfx_TransparentSprite_NoClip(creativeinventory,62,52);
                    gfx_TransparentSprite_NoClip(scroller,62+175,52+18+scrolly*scrollfactor);
                    for(int i = 0; i < 5; i++) {
                        if (hotbar[i] != 0) {
                            if (istransparent(hotbar[i])) {
                                gfx_TransparentSprite(blocks[hotbar[i]],62+45+i*18,52+112);
                            } else {
                                gfx_Sprite(blocks[hotbar[i]],62+45+i*18,52+112);
                            }
                        }
                    }
                    for(int y = 0; y < 5; y++) {
                        for(int x = 0; x < 9; x++) {
                            thistile = x+1+(9*(y+scrolly));
                            if ((blocks[thistile] != 0) && (thistile < 121)) {
                                if (istransparent(thistile)) {
                                    gfx_TransparentSprite_NoClip(blocks[thistile],62+9+x*18,52+18*(y+1));
                                } else {
                                    gfx_Sprite_NoClip(blocks[thistile],62+9+x*18,52+18*(y+1));
                                }
                            }
                        }
                    }
                    if (invcursorx != 9) {
                        if (invcursory >= 2 && invcursory < 7) {
                            if (selectedblock) {
                                gfx_TransparentSprite_NoClip(shading,(invcursorx)*18+62+9+5,(invcursory-2)*18+52+18+5);
                                if (istransparent(selectdata)) {
                                    gfx_TransparentSprite_NoClip(blocks[selectdata],(invcursorx)*18+62+9+3,(invcursory-2)*18+52+18+3);
                                } else {
                                    gfx_Sprite_NoClip(blocks[selectdata],(invcursorx)*18+62+9+3,(invcursory-2)*18+52+18+3);
                                }
                            }
                            gfx_TransparentSprite(cursor,(invcursorx)*18+62+9,(invcursory-2)*18+52+18);
                        } else if (invcursory == 7) {
                            if (selectedblock) {
                                gfx_TransparentSprite_NoClip(shading,(invcursorx)*18+62+9+5,(invcursory-2)*18+52+22+5);
                                if (istransparent(selectdata)) {
                                    gfx_TransparentSprite_NoClip(blocks[selectdata],(invcursorx)*18+62+9+3,(invcursory-2)*18+52+22+3);
                                } else {
                                    gfx_Sprite_NoClip(blocks[selectdata],(invcursorx)*18+62+9+3,(invcursory-2)*18+52+22+3);
                                }
                            }
                            gfx_TransparentSprite(cursor,(invcursorx)*18+62+9,(invcursory-2)*18+52+22);
                        }
                    } else {
                        gfx_SetColor(255);
                        gfx_Rectangle(62+174,52+18+scrolly*scrollfactor-1,14,17);
                        gfx_SetColor(0);
                        gfx_Rectangle(62+173,52+18+scrolly*scrollfactor-2,16,19);
                    }
                } else {
                    gfx_TransparentSprite_NoClip(inventory,72,37);
                    for(int i = 0; i < 5; i++) {
                        if (hotbar[i] != 0) {
                            gfx_Sprite_NoClip(blocks[hotbar[i]],72+44+i*18,37+142);
                        }
                    }
                    for(int i = 0; i < 5; i++) {
                        if (hotbarquant[i] > 1) {
                            gfx_SetTextXY(72+44+i*18+4,37+142+10);
                            gfx_PrintUInt(hotbarquant[i],0);
                        }
                    }
                    for(int y = 0; y < 3; y++) {
                        for(int x = 0; x < 9; x++) {
                            thistile = inventorydata[x+(y*9)];
                            if (blocks[thistile] != 0) {
                                if (istransparent(thistile)) {
                                    gfx_TransparentSprite_NoClip(blocks[thistile],72+8+x*18,37+84+18*(y));
                                } else {
                                    gfx_Sprite_NoClip(blocks[thistile],72+8+x*18,37+84+18*(y));
                                }
                            }
                        }
                    }
                    if (invcursory >= 4 && invcursory <= 6) {
                        if (selectedblock) {
                            gfx_TransparentSprite_NoClip(shading,invcursorx*18+72+8+5,invcursory*18+37+12+5);
                            if (istransparent(selectdata)) {
                                gfx_TransparentSprite_NoClip(blocks[selectdata],invcursorx*18+72+8+3,invcursory*18+37+12+3);
                            } else {
                                gfx_Sprite_NoClip(blocks[selectdata],invcursorx*18+72+8+3,invcursory*18+37+12+3);
                            }
                        }
                        gfx_TransparentSprite(cursor,invcursorx*18+72+8,invcursory*18+37+12);
                    } else if (invcursory < 4 && invcursorx == 0) {
                        gfx_TransparentSprite(cursor,invcursorx*18+72+8,invcursory*18+37+8);
                    } else if (invcursory < 4 && invcursorx != 0) {
                        gfx_TransparentSprite(cursor,invcursorx*18+72+8,invcursory*18+37+8);
                    } else if (invcursory == 7) {
                        if (selectedblock) {
                            gfx_TransparentSprite_NoClip(shading,(invcursorx+2)*18+72+8+5,invcursory*18+37+16+5);
                            if (istransparent(selectdata)) {
                                gfx_TransparentSprite_NoClip(blocks[selectdata],(invcursorx+2)*18+72+8+3,invcursory*18+37+16+3);
                            } else {
                                gfx_Sprite_NoClip(blocks[selectdata],(invcursorx+2)*18+72+8+3,invcursory*18+37+16+3);
                            }
                        }
                        gfx_TransparentSprite(cursor,(invcursorx+2)*18+72+8,invcursory*18+37+16);
                    }
                }
            } else if (hotbar[currentslot] == 120) {
                gfx_ScaledTransparentSprite_NoClip(smallmap,45,5,10,10);
                // gfx_SetColor(255);
                // gfx_FillRectangle(50,56,220,128);
                gfx_SetColor(0);
                for(int x = 0; x < worldwidth; x++) {
                    for(int y = 0; y < worlddepth; y++) {
                        if (worlda[x][y] || worldb[x][y]) {
                            gfx_SetPixel(x+50,y+55);
                        }
                    }
                }
                if (playerleft) {
                    gfx_TransparentSprite_NoClip(playericonleft,playerx+50-3,playery+55-2);
                } else {
                    gfx_TransparentSprite_NoClip(playericonright,playerx+50-4,playery+55-3);
                }
                for(int i = 0; i < 5; i++) {
                    gfx_Sprite_NoClip(hotbarslot,110+i*20,210);
                    if (hotbar[i] != 0) {
                        if (istransparent(hotbar[i])) {
                            gfx_TransparentSprite_NoClip(blocks[hotbar[i]],112+i*20,212);
                        } else {
                            gfx_Sprite_NoClip(blocks[hotbar[i]],112+i*20,212);
                        }
                    }
                }
                gfx_TransparentSprite_NoClip(selectslot,109+currentslot*20,209);
                if (!creativemode) {
                    for(int i = 0; i < 5; i++) {
                        if (hotbarquant[i] > 1) {
                            gfx_SetTextXY(72+44+i*20,212+10);
                            gfx_PrintUInt(hotbarquant[i],0);
                        }
                    }
                }
            }
            gfx_SwapDraw();
            /*
                inventory cursor also do be weird.
                S|012345678
                -|---------
                0|#
                1|####
                2|###
                3|#
                4|#########
                5|#########
                6|#########
                7|  #####
                creative mode has to include scrollbar at invcursorx = 9
                C|0123456789
                -|----------
                0|
                1|
                2|##########
                3|##########
                4|##########
                5|##########
                6|##########
                7|  #####
            */
            kb_Scan();
            if (inventoryopen) {
                if (kb_Data[7] & kb_Left) {invcursorx -= 1;}
                if (kb_Data[7] & kb_Right) {invcursorx += 1;}
                if (invcursorx == 9) {
                    if ((kb_Data[7] & kb_Up) && scrolly > 0) {scrolly -= 1;}
                    if ((kb_Data[7] & kb_Down) && scrolly < (int)ceil(121/9)-1) {scrolly += 1;}
                } else {
                    if (kb_Data[7] & kb_Up) {invcursory -= 1;}
                    if (kb_Data[7] & kb_Down) {invcursory += 1;}
                }
                if (creativemode) {
                    if ((kb_Data[1] & kb_2nd) && !sndkeypress) {
                        if (selectedblock) {
                            selectedblock = false;
                            if (invcursory == 7) {
                                hotbar[invcursorx-2] = selectdata;
                            }
                            selectdata = 0;
                        } else {
                            if (invcursory != 7) {
                                if ((invcursorx)+1+(9*(invcursory-2+scrolly)) != 0 && (invcursorx)+1+(9*(invcursory-2+scrolly)) <= 120) {
                                    selectdata = (invcursorx)+1+(9*(invcursory-2+scrolly));
                                }
                            } else {
                                if (hotbar[invcursorx-2] != 0) {
                                    selectdata = hotbar[invcursorx-2];
                                    hotbar[invcursorx-2] = 0;
                                }
                            }
                            selectedblock = true;
                        }
                        sndkeypress = true;
                    }
                    if (invcursory >= 2 && invcursory <= 6 && invcursorx > 9) {invcursorx = 9;}
                    if (invcursory == 7 && invcursorx > 6) {invcursorx = 6;}
                    if (invcursory == 7 && invcursorx < 2) {invcursorx = 2;}
                    if (invcursory > 7) {invcursory = 7;}
                    if (invcursory < 2) {invcursory = 2;}
                    if (!(kb_Data[1] & kb_2nd)) {sndkeypress = false;} 
                } else {
                    if (kb_Data[1] & kb_2nd) {
                        if (selectedblock) {
                            if (invcursory == 7) {
                                hotbar[invcursorx-2] = selectdata;
                            } else {
                                inventorydata[invcursorx+(invcursory-4)*9] = selectdata;
                            }
                            selectdata = 0;
                            selectedblock = false;
                        } else {
                            selectdata = inventorydata[invcursorx+(invcursory-4)*9];
                            inventorydata[invcursorx+(invcursory-4)*9] = 0;
                            selectedblock = true;
                        }
                        sndkeypress = true;
                    }
                    if (invcursory <= 6 && invcursory >= 4 && invcursorx > 8) {invcursorx = 0;}
                    if (invcursory == 7 && invcursorx > 4) {invcursorx = 4;}
                    if ((invcursory == 0 || invcursory == 3) && invcursorx != 0) {invcursorx = 0;}
                    if (invcursory == 1 && invcursorx > 3) {invcursorx = 0;}
                    if (invcursory == 2 && invcursorx > 2) {invcursorx = 0;}
                    if (invcursory > 7) {invcursory = 0;}
                    if (!(kb_Data[1] & kb_2nd)) {sndkeypress = false;} 
                }
            } else {
                playery += playeryvel;
                playerx += playerxvel;
                playerxvel *= 0.5; 
                playeryvel *= 0.8;
                inblock = worldb[(int)fabsf(floorf(playerx))][(int)fabsf(floorf(playery+0.6))];
                onground = !isnonblocking(worlda[(int)fabsf(floorf(playerx))][(int)fabsf(floorf(playery+0.9))]);
                hitceil = !isnonblocking(worlda[(int)playerx][(int)(playery-1)]);
                blockleft = (bool)worlda[(int)floorf(playerx-0.5)][(int)floorf(playery)] || (bool)worlda[(int)floorf(playerx-0.5)][(int)floorf(playery)-1];
                blockright = (bool)worlda[(int)floorf(playerx+0.5)][(int)floorf(playery)] || (bool)worlda[(int)floorf(playerx+0.5)][(int)floorf(playery)-1];
                if (blockleft) {
                    if (worlda[(int)floorf(playerx-0.5)][(int)floorf(playery)] != 0) {
                        blockidleft = worlda[(int)floorf(playerx-0.5)][(int)floorf(playery)];
                    } else {
                        blockidleft = worlda[(int)floorf(playerx-0.5)][(int)floorf(playery)-1];
                    }
                }
                if (blockright) {
                    if (worlda[(int)floorf(playerx+0.5)][(int)floorf(playery)] != 0) {
                        blockidright = worlda[(int)floorf(playerx+0.5)][(int)floorf(playery)];
                    } else {
                        blockidright = worlda[(int)floorf(playerx+0.5)][(int)floorf(playery)-1];
                    }
                }
                if (inblock == 40 || worlda[(int)fabsf(floorf(playerx))][(int)fabsf(floorf(playery+0.6))] == 40) {
                    onladder = true;
                } else {
                    onladder = false;
                }
                if (!onground && !onladder) {playeryvel += 0.1;} else {playeryvel = 0;}
                didloop = false;
                if (onground) { 
                    do {
                        playery -= 0.1;
                        onground = !isnonblocking(worlda[(int)fabsf(floorf(playerx))][(int)fabsf(floorf(playery+0.9))]);
                    } while (onground);
                    didloop = true;
                }
                if (hitceil) {
                    do {
                        playery += 0.1;
                        hitceil = !isnonblocking(worlda[(int)playerx][(int)(playery-1)]);
                    } while (hitceil);
                }
                if ((kb_Data[7] & kb_Up)) {
                    if (onladder) {
                        playeryvel = 0;
                        playery -= 0.2;
                    } else if (didloop) {
                        playeryvel = -0.5;
                    }
                }
                if (!(kb_Data[7] & kb_Up) && onladder) {
                    playeryvel = 0;
                    playery += 0.2;
                }
                if (kb_Data[7] & kb_Left) {
                    if (!blockleft || isnonblocking(blockidleft)) playerxvel -= 0.1;
                    playerleft = true;
                }
                if (kb_Data[7] & kb_Right) {
                    if (!blockright || isnonblocking(blockidright)) playerxvel += 0.1;
                    playerleft = false;
                }
                if (kb_Data[4] & kb_Stat) {cursorh -= 1;}
                if (kb_Data[4] & kb_Prgm) {cursorh += 1;}
                if (kb_Data[1] & kb_Yequ) {currentslot = 0;}
                if (kb_Data[1] & kb_Window) {currentslot = 1;}
                if (kb_Data[1] & kb_Zoom) {currentslot = 2;}
                if (kb_Data[1] & kb_Trace) {currentslot = 3;}
                if (kb_Data[1] & kb_Graph) {currentslot = 4;}
                if (kb_Data[1] & kb_2nd && !sndkeypress) {
                    if (creativemode) {
                        if (worlda[cursorx][cursory-1] == 0) {
                            worldb[cursorx][cursory-1] = 0;
                        } else {
                            worlda[cursorx][cursory-1] = 0;
                        }
                    } else {
                        if (worlda[cursorx][cursory-1] == 0) {
                            uint8_t pickedslot = 0;
                            for(pickedslot = 0; inventorydata[pickedslot] != 0; pickedslot++)
                            inventorydata[pickedslot] = worldb[cursorx][cursory-1];
                            worldb[cursorx][cursory-1] = 0;
                        } else {
                            bool haspickedslot = false;
                            uint8_t pickedslot = 0;
                            do {
                                if (inventorydata[pickedslot] == 0) {
                                    haspickedslot = true;
                                }
                                pickedslot++;
                            } while (!haspickedslot);
                            inventorydata[pickedslot] = worlda[cursorx][cursory-1];
                            worlda[cursorx][cursory-1] = 0;
                        }
                    }
                    sndkeypress = true;
                }
                if ((kb_Data[2] & kb_Alpha) && (hotbar[currentslot] != 0) && !alphakeypress && (hotbar[currentslot] < 96)) {
                    if (isnonblocking(hotbar[currentslot])) {
                        worlda[cursorx][cursory-1] = hotbar[currentslot];
                    } else {
                        if (worldb[cursorx][cursory-1] == 0) {
                            worldb[cursorx][cursory-1] = hotbar[currentslot];
                        } else if (worlda[cursorx][cursory-1] == 0) {
                            worlda[cursorx][cursory-1] = hotbar[currentslot];
                        }
                    }
                    alphakeypress = true;
                }
            }
            if (kb_Data[1] & kb_Mode && !modekeypress) {
                if (inventoryopen) {
                    inventoryopen = false;
                } else {
                    inventoryopen = true;
                }
                modekeypress = true;
            }
            if (!(kb_Data[1] & kb_2nd)) {sndkeypress = false;}
            if (!(kb_Data[2] & kb_Alpha)) {alphakeypress = false;}
            if (!(kb_Data[1] & kb_Mode)) {modekeypress = false;}
            if (playerx < 11) {playerx = 11;}
            if (playerx > 209) {playerx = 209;}
            if (playery < 7) {playery = 7;}
            if (playery > 125) {playery = 125;}

            xoffset = fabsf(playerx - roundf(playerx));
            yoffset = fabsf(playery - roundf(playery));
            if (cursorh > 5) {cursorh = 5;}
            if (cursorh < 0) {cursorh = 0;}
            cursory = fabsf(playery + (cursorh - 2) - yoffset);
            if (cursorh == 0) {
                cursory = fabsf(playery + (cursorh - 1) - yoffset);
            }
            if (cursorh == 5) {
                cursory = fabsf(playery + (cursorh - 3) - yoffset);
            }
            if (cursorh >= 1 && cursorh <= 4) {
                cursorx = fabsf(playerx + 0-(2*playerleft-1) - xoffset);
            } else {
                cursorx = fabsf(playerx - xoffset);
            }
        } while (!(kb_Data[6] == kb_Clear));
        // #ifdef nopano
        //     for(uint8_t x = 0; x < 20; x++) {
        //         for(uint8_t y = 0; y < 15; y++) {
        //             gfx_Sprite(dirt,x*16,y*16);
        //         }
        //     }
        //     gfx_Sprite(logo,48,48);
        // #else
        //     gfx_ScaledSprite_NoClip(panorama,0,0,2,2);
        //     gfx_TransparentSprite(logo_transparent,48,48);
        // #endif
        // gfx_PrintStringXY("Saving world",160-(gfx_GetStringWidth("Saving world")/2),150);
        // gfx_PrintStringXY("This can take over 30 seconds",160-(gfx_GetStringWidth("This can take over 30 seconds")/2),165);
        // gfx_SwapDraw();
        // uint8_t exportdata = ti_Open("MCWORLDA","w+");
        // void* writedata = 0;
        // if (exportdata != 0) {
        //     ti_Rewind(exportdata);
        //     for (int y = 0; y < worlddepth; y++) {
        //         for (int x = 0; x < worldwidth; x++) {
        //             writedata = &worlda[x][y];
        //             ti_Write(writedata,1,1,exportdata);
        //         }
        //     }
        //     for (int y = 0; y < worlddepth; y++) {
        //         for (int x = 0; x < worldwidth; x++) {
        //             writedata = &worldb[x][y];
        //             ti_Write(writedata,1,1,exportdata);
        //         }
        //     }
        //     uint8_t playerxsave = (int)playerx;
        //     uint8_t playerysave = (int)playery;
        //     writedata = &playerxsave;
        //     ti_Write(writedata,1,1,exportdata);
        //     writedata = &playerysave;
        //     ti_Write(writedata,1,1,exportdata);
        // }
        // ti_Close(exportdata);
        // exportdata = ti_Open("MCWORLDB","w+");
        // if (exportdata != 0) {
        //     ti_Rewind(exportdata);
        //     for (int y = 0; y < worldwidth; y++) {
        //         for (int x = 0; x < worldwidth; x++) {
        //             writedata = &worldb[x][y];
        //             ti_Write(writedata,1,1,exportdata);
        //         }
        //     }
        //     uint8_t playerxsave = (int)playerx;
        //     uint8_t playerysave = (int)playery;
        //     writedata = &playerxsave;
        //     ti_Write(writedata,1,1,exportdata);
        //     writedata = &playerysave;
        //     ti_Write(writedata,1,1,exportdata);
        // }
        // ti_Close(exportdata);
        // exportdata = ti_Open("MCWORLDA","r+");
        // ti_SetArchiveStatus(true,exportdata);
        // ti_Close(exportdata);
        // exportdata = ti_Open("MCWORLDB","r+");
        // ti_SetArchiveStatus(true,exportdata);
        // ti_Close(exportdata);
        // uint8_t getthethingarchived = ti_Open("MCEWORLD","r");
        // ti_SetArchiveStatus(true,getthethingarchived);
        // ti_Close(getthethingarchived);
    }
    gfx_End();
    return 0;
}