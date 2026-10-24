#ifndef CTUI_TUI_LAYOUT_H
#define CTUI_TUI_LAYOUT_H

#define CTUI_OBJECT_X_GROW (1 << 0)
#define CTUI_OBJECT_Y_GROW (1 << 1)
#define CTUI_OBJECT_X_FIT (1 << 2)
#define CTUI_OBJECT_Y_FIT (1 << 3)
#define CTUI_OBJECT_VISIBLE (1 << 4)
#define CTUI_OBJECT_ROOT_PARENT (1 << 5)

struct ctui_object
{
  size_t x_offset;
  size_t y_offset;
  size_t width;
  size_t height;
  sds_da_index_t parent_id;
  sds_da_index_t *children_id_arr;
  unsigned int flags;
};
struct ctui_ui_state
{
  struct sds_id_da object_manager;
  int rows;
  int columns;
};
int ctui_ui_state_create(struct ctui_ui_state *create);

#endif