
void *cf_add_string(char *k, char *s)
{
    cflist *node;

    node = malloc(sizeof(*node));
    assert_that(node, is_not_null);

    node->key = k;
    node->string = s;

    node->next = scf_list_first;
    scf_list_first = node;
    
    return node;
}
