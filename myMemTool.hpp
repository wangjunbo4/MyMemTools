/*
 * @Description: 
 * @Author: Gtylcara
 * @Github: https://github.com/wangjunbo4
 * @Date: 2022-05-26 12:01:36
 * @LastEditors: Gtylcara
 * @LastEditTime: 2022-05-26 19:21:34
 * @FilePath: /memcheck/myMemTool.hpp
 */

#ifndef _MYMEMTOOL_HPP_
#define _MYMEMTOOL_HPP_

#include <string>
#include <iostream>
#include <mutex>
#include <cstring>
#include <memory>
#include <vector>

struct Map_t
{
    void *p;
    int cnt;
    struct Map_t *next;
};

class MyMemTools
{
public:

    MyMemTools()
    {
    }

    static bool In(void * p)
    {
        std::unique_lock<std::mutex> lock(getLock());
        auto memoryMap = getMap();
        std::cout << "In: " << p << std::endl;
        
        while (memoryMap != nullptr)
        {
            if (memoryMap->p == p)
            {
                memoryMap->cnt = 1;
                return true;
            }
            memoryMap = memoryMap->next;
        }
        
        auto temp = newNode();
        temp->p = p;
        temp->cnt = 1;
        insertNode(getMap(), temp);

        return true;

    }

    static bool Out(void *p)
    {
        std::unique_lock<std::mutex> lock(getLock());
        auto memoryMap = getMap();
        std::cout << "Out: " << p << std::endl;
        
        while (memoryMap != nullptr)
        {
            if (memoryMap->p == p)
            {
                memoryMap->cnt = 0;
                return true;
            }
            memoryMap = memoryMap->next;
        }
        return false;
    }

    static void result()
    {
        auto memoryMap = getMap();
        std::cout << "result: " << std::endl;
        int cnt = 0;

        while (memoryMap != nullptr)
        {
            if (memoryMap->cnt != 0)
            {
                std::cout << "memory leak: " << memoryMap->p << std::endl;
                cnt++;
            }
            memoryMap = memoryMap->next;
        }

        if (cnt == 0)
        {
            std::cout << "no memory leak" << std::endl;
        }
        deleteAll(getMap());
    }

private:
    static inline Map_t *getMap()
    {
        static Map_t *memoryMap = newNode();
        return memoryMap;
    }

    static inline Map_t * newNode()
    {
        Map_t *p = (struct Map_t *)malloc(sizeof(Map_t));
        p->p = nullptr;
        p->cnt = 0;
        p->next = nullptr;
        return p;
    }

    static void deleteNode(Map_t *p, Map_t *head)
    {
        if (p == head)
        {
            head = p->next;
            free(p);
            return;
        }

        Map_t *q = head;

        while (q->next != p)
        {
            q = q->next;
        }
        q->next = p->next;
        free(p);
    }

    static void deleteAll(Map_t *p)
    {
        while (p != nullptr)
        {
            auto temp = p;
            p = p->next;
            free(temp);
        }
    }

    static inline void insertNode(Map_t *head, Map_t *p)
    {
        if (head == nullptr)
        {
            head = p;
            return;
        }
        Map_t *temp = head->next;
        head->next = p;
        p->next = temp;
    }

    static inline std::mutex& getLock()
    {
        static std::mutex m;
        return m;
    }
};

void* operator new(std::size_t size) { 
    auto p = malloc(size);
    if (MyMemTools::In(p))
    {
        return operator new(size, p);
    }
    else
    {
        throw(std::bad_alloc());
    }
}

void operator delete(void *p) { 
    std::cout << "deallocate\n";
    if (MyMemTools::Out(p))
    {
        free(p);
    }
}

#endif