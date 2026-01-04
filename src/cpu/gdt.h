//
// Created by andy on 1/3/26.
//

#pragma once

void init_gdt();

#define GDT_NULL 0
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE 0x18
#define GDT_USER_DATA 0x20
