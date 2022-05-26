/*
 * @Description: 
 * @Author: Gtylcara
 * @Github: https://github.com/wangjunbo4
 * @Date: 2022-05-26 12:01:36
 * @LastEditors: Gtylcara
 * @LastEditTime: 2022-05-26 23:22:17
 * @FilePath: /memcheck/myMemTool.hpp
 */
#pragma once

#ifndef _MYMEMTOOL_HPP_
#define _MYMEMTOOL_HPP_

#include <cstring>
#include <iostream>
#include <mutex>
#include <cstring>
#include <memory>
#include <vector>

struct Map_t
{
    void *p;
    int cnt;
    char *file;
    int line;
    struct Map_t *next;
};

class MyMemTools
{
public:

    MyMemTools()
    {
    }

    static bool In(void * p, const char* file, int line)
    {
        std::unique_lock<std::mutex> lock(getLock());
        auto memoryMap = getMap();
        
        while (memoryMap != nullptr)
        {
            if (memoryMap->p == p)
            {
                memoryMap->cnt = 1;
                memoryMap->file = (char *)malloc(strlen(file) + 1);
                std::strcpy(memoryMap->file, file);
                memoryMap->line = line;
                return true;
            }
            memoryMap = memoryMap->next;
        }
        
        auto temp = newNode();
        temp->p = p;
        temp->cnt = 1;
        temp->file = (char *)malloc(strlen(file) + 1);
        std::strcpy(temp->file, file);
        temp->line = line;
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
                free(memoryMap->file);
                memoryMap->file = nullptr;
                memoryMap->line = 0;
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
                std::cout << "memory leak: " << memoryMap->p << " in file: " << memoryMap->file << " line: " << memoryMap->line << std::endl;
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
        p->file = nullptr;
        p->line = 0;
        return p;
    }

    static void deleteNode(Map_t *p, Map_t *head)
    {
        if (p == head)
        {
            head = p->next;
            free(p->file);
            free(p);
            return;
        }

        Map_t *q = head;

        while (q->next != p)
        {
            q = q->next;
        }
        q->next = p->next;
        free(p->file);
        free(p);
    }

    static void deleteAll(Map_t *p)
    {
        while (p != nullptr)
        {
            auto temp = p;
            p = p->next;
            free(temp->file);
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

inline void* operator new(std::size_t size, const char *file, int line)
{
    auto p = malloc(size);
    std::cout << "new: " << p << " in file: " << file << " line: " << line << std::endl;
    if (MyMemTools::In(p, file, line))
    {
        return p;
    }
    else
    {
        free(p);
        throw(std::bad_alloc());
    }
}


inline void operator delete(void *p) { 
    std::cout << "deallocate\n";
    if (MyMemTools::Out(p))
    {
        free(p);
    }
}

#endif