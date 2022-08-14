#ifndef __LINKLIST_H__
#define __LINKLIST_H__
#include <stdio.h>
#include <malloc.h>

struct linklist_node{
    int data;
    struct linklist_node* next;
};

int Member(struct linklist_node** _head, int _val); /* if _val in list, return 1, else 0 */
int Insert(struct linklist_node** _head, int _val); /* insert a new _val, if success, return 1, else 0 */
int Delete(struct linklist_node** _head, int _val); /* delete a _val in list, if success, return 1, else 0 */
void Layout(struct linklist_node** _head); /* print the whole list */

void Layout(struct linklist_node** _head){
    struct linklist_node* _cur = *_head;

    while(_cur != NULL){
        printf("%d->", _cur->data);
        _cur = _cur->next;
    }
    printf("NULL\n");
} /* Layout */

int Member(struct linklist_node** _head, int _val){
    struct linklist_node* _cur = *_head;

    while(_cur != NULL && _cur->data < _val)
        _cur = _cur->next;
    
    if (_cur == NULL || _cur->data > _val)
        return 0;
    else
        return 1;
} /* Member */

int Insert(struct linklist_node** _head, int _val){
    struct linklist_node* _cur = *_head;
    struct linklist_node* _pre = NULL;

    while(_cur != NULL && _cur->data < _val){
        _pre = _cur;
        _cur = _cur->next;
    }

    if (_cur == NULL || _cur->data > _val){
        struct linklist_node* _tmp = (struct linklist_node*)malloc(sizeof(struct linklist_node));
        _tmp->data = _val;
        _tmp->next = _cur;
        if (_pre == NULL)
            *_head = _tmp;
        else
            _pre->next = _tmp;

        return 1;
    }
    else
        return 0;
} /* Insert */

int Delete(struct linklist_node** _head, int _val){
    struct linklist_node* _cur = *_head;
    struct linklist_node* _pre = NULL;

    while(_cur != NULL && _cur->data < _val){
        _pre = _cur;
        _cur = _cur->next;
    }

    if (_cur != NULL && _cur->data == _val){
        if (_pre == NULL){
            *_head = _cur->next;
            free(_cur);
        }
        else{
            _pre->next = _cur->next;
            free(_cur);
        }
        return 1;
    }
    else
        return 0;
} /* Delete */

#endif /* __LINKLIST_H__ */
