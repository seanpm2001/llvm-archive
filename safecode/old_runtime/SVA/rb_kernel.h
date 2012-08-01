typedef struct rb_node_s
{
  struct rb_node_s * rb_parent;
  int rb_color;
#define RB_RED          0
#define RB_BLACK        1
  struct rb_node_s * rb_right;
  struct rb_node_s * rb_left;
}
  rb_node_t;

typedef struct rb_root_s
{
  struct rb_node_s * rb_node;
}
  rb_root_t;

#define RB_ROOT (rb_root_t) { NULL, }
#define rb_entry(ptr, type, member)                                     \
  ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

static inline void rb_link_node(rb_node_t * node, rb_node_t * parent, rb_node_t ** rb_link)
{
  node->rb_parent = parent;
  node->rb_color = RB_RED;
  node->rb_left = node->rb_right = 0;

  *rb_link = node;
}

static void __rb_rotate_left(rb_node_t * node, rb_root_t * root)
{
  rb_node_t * right = node->rb_right;

  if ((node->rb_right = right->rb_left))
    right->rb_left->rb_parent = node;
  right->rb_left = node;

  if ((right->rb_parent = node->rb_parent))
    {
      if (node == node->rb_parent->rb_left)
        node->rb_parent->rb_left = right;
      else
        node->rb_parent->rb_right = right;
    }
  else
    root->rb_node = right;
  node->rb_parent = right;
}

static void __rb_rotate_right(rb_node_t * node, rb_root_t * root)
{
  rb_node_t * left = node->rb_left;

  if ((node->rb_left = left->rb_right))
    left->rb_right->rb_parent = node;
  left->rb_right = node;

  if ((left->rb_parent = node->rb_parent))
    {
      if (node == node->rb_parent->rb_right)
        node->rb_parent->rb_right = left;
      else
        node->rb_parent->rb_left = left;
    }
  else
    root->rb_node = left;
  node->rb_parent = left;
}

static void rb_insert_color(rb_node_t * node, rb_root_t * root)
{
  rb_node_t * parent, * gparent;

  while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
    {
      gparent = parent->rb_parent;

      if (parent == gparent->rb_left)
        {
          {
            register rb_node_t * uncle = gparent->rb_right;
            if (uncle && uncle->rb_color == RB_RED)
              {
                uncle->rb_color = RB_BLACK;
                parent->rb_color = RB_BLACK;
                gparent->rb_color = RB_RED;
                node = gparent;
                continue;
              }
          }

          if (parent->rb_right == node)
            {
              register rb_node_t * tmp;
              __rb_rotate_left(parent, root);
              tmp = parent;
              parent = node;
              node = tmp;
            }

          parent->rb_color = RB_BLACK;
          gparent->rb_color = RB_RED;
          __rb_rotate_right(gparent, root);
        } else {
        {
          register rb_node_t * uncle = gparent->rb_left;
          if (uncle && uncle->rb_color == RB_RED)
            {
              uncle->rb_color = RB_BLACK;
              parent->rb_color = RB_BLACK;
              gparent->rb_color = RB_RED;
              node = gparent;
              continue;
            }
        }

        if (parent->rb_left == node)
          {
            register rb_node_t * tmp;
            __rb_rotate_right(parent, root);
            tmp = parent;
            parent = node;
            node = tmp;
          }

        parent->rb_color = RB_BLACK;
        gparent->rb_color = RB_RED;
        __rb_rotate_left(gparent, root);
      }
    }

  root->rb_node->rb_color = RB_BLACK;
}

static void __rb_erase_color(rb_node_t * node, rb_node_t * parent,
                             rb_root_t * root)
{
  rb_node_t * other;

  while ((!node || node->rb_color == RB_BLACK) && node != root->rb_node)
    {
      if (parent->rb_left == node)
        {
          other = parent->rb_right;
          if (other->rb_color == RB_RED)
            {
              other->rb_color = RB_BLACK;
              parent->rb_color = RB_RED;
              __rb_rotate_left(parent, root);
              other = parent->rb_right;
            }
          if ((!other->rb_left ||
               other->rb_left->rb_color == RB_BLACK)
              && (!other->rb_right ||
                  other->rb_right->rb_color == RB_BLACK))
            {
              other->rb_color = RB_RED;
              node = parent;
              parent = node->rb_parent;
            }
          else
            {
              if (!other->rb_right ||
                  other->rb_right->rb_color == RB_BLACK)
                {
                  register rb_node_t * o_left;
                  if ((o_left = other->rb_left))
                    o_left->rb_color = RB_BLACK;
                  other->rb_color = RB_RED;
                  __rb_rotate_right(other, root);
                  other = parent->rb_right;
                }
              other->rb_color = parent->rb_color;
              parent->rb_color = RB_BLACK;
              if (other->rb_right)
                other->rb_right->rb_color = RB_BLACK;
              __rb_rotate_left(parent, root);
              node = root->rb_node;
              break;
            }
        }
      else
        {
          other = parent->rb_left;
          if (other->rb_color == RB_RED)
            {
              other->rb_color = RB_BLACK;
              parent->rb_color = RB_RED;
              __rb_rotate_right(parent, root);
              other = parent->rb_left;
            }
          if ((!other->rb_left ||
               other->rb_left->rb_color == RB_BLACK)
              && (!other->rb_right ||
                  other->rb_right->rb_color == RB_BLACK))
            {
              other->rb_color = RB_RED;
              node = parent;
              parent = node->rb_parent;
            }
          else
            {
              if (!other->rb_left ||
                  other->rb_left->rb_color == RB_BLACK)
                {
                  register rb_node_t * o_right;
                  if ((o_right = other->rb_right))
                    o_right->rb_color = RB_BLACK;
                  other->rb_color = RB_RED;
                  __rb_rotate_left(other, root);
                  other = parent->rb_left;
                }
              other->rb_color = parent->rb_color;
              parent->rb_color = RB_BLACK;
              if (other->rb_left)
                other->rb_left->rb_color = RB_BLACK;
              __rb_rotate_right(parent, root);
              node = root->rb_node;
              break;
            }
        }
    }
  if (node)
    node->rb_color = RB_BLACK;
}

static void rb_erase(rb_node_t * node, rb_root_t * root)
{
  rb_node_t * child, * parent;
  int color;

  if (!node->rb_left)
    child = node->rb_right;
  else if (!node->rb_right)
    child = node->rb_left;
  else
    {
      rb_node_t * old = node, * left;

      node = node->rb_right;
      while ((left = node->rb_left))
        node = left;
      child = node->rb_right;
      parent = node->rb_parent;
      color = node->rb_color;

      if (child)
        child->rb_parent = parent;
      if (parent)
        {
          if (parent->rb_left == node)
            parent->rb_left = child;
          else
            parent->rb_right = child;
        }
      else
        root->rb_node = child;

      if (node->rb_parent == old)
        parent = node;
      node->rb_parent = old->rb_parent;
      node->rb_color = old->rb_color;
      node->rb_right = old->rb_right;
      node->rb_left = old->rb_left;

      if (old->rb_parent)
        {
          if (old->rb_parent->rb_left == old)
            old->rb_parent->rb_left = node;
          else
            old->rb_parent->rb_right = node;
        } else
        root->rb_node = node;

      old->rb_left->rb_parent = node;
      if (old->rb_right)
        old->rb_right->rb_parent = node;
      goto color;
    }

  parent = node->rb_parent;
  color = node->rb_color;

  if (child)
    child->rb_parent = parent;
  if (parent)
    {
      if (parent->rb_left == node)
        parent->rb_left = child;
      else
        parent->rb_right = child;
    }
  else
    root->rb_node = child;

 color:
  if (color == RB_BLACK)
    __rb_erase_color(child, parent, root);
}
