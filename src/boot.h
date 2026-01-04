//
// Created by andy on 1/3/26.
//

#pragma once

#include <limine.h>
#include <stdbool.h>


bool verify_limine_base_revision();

struct limine_framebuffer_response* get_limine_framebuffer();
struct limine_memmap_response* get_limine_memmap();
struct limine_date_at_boot_response* get_limine_date_at_boot();
struct limine_executable_address_response* get_limine_executable_address();
struct limine_rsdp_response* get_limine_rsdp();
struct limine_hhdm_response* get_limine_hhdm();
