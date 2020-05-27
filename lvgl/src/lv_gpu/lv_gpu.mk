CSRCS += lv_gpu_stm32_dma2d.c

DEPPATH += --dep-path $(LVGL_DIR)/src/lv_gpu
VPATH += :$(LVGL_DIR)/src/lv_gpu

CFLAGS += "-I$(LVGL_DIR)/src/lv_gpu"
