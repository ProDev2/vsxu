#pragma once

#include "vsx_filesystem_tree_writer.h"

class vsx_filesystem_tree_serialize_string
{

  static vsx_string<> serialize_node(vsx_filesystem_tree_node* node)
  {
    vsx_string<> result;
    foreach (node->children, i)
      result += node->children[i]->name+"\n";

    foreach (node->children, i)
      result += serialize_node(node->children[i]);

    return result;
  }

public:

  static vsx_string<> serialize(vsx_filesystem_tree_writer& writer)
  {
    return serialize_node(&writer.root_node);
  }
};