#ifdef GTEST_DEBUG

#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <deque>
#include <inc/klibs/deque.h>

using std::cout;

TEST(LargeAdd, AWholeTest) {
    Deque<int> q;
    std::deque<int> ans;

    SCOPED_TRACE("Testing Sequential Read...");
    for(int i=0; i<1024; i++)
    {
        int x = rand();
        q.push_back(x);
        ans.push_back(x);
    }

    for(int i=0; i<1024; i++)
        ASSERT_TRUE(q[i]==ans[i]);


    SCOPED_TRACE("Testing Copying...");
    Deque<int> q2 = q;
    std::deque<int> ans2 = ans;

    for(int i=0; i<1024; i++)
        ASSERT_TRUE(q2[i]==ans2[i]);

    for(int i=0; i<1024; i++)
    {
        ASSERT_TRUE(*q2.front() == ans2.front());
        ASSERT_TRUE(*q2.back() == ans2.back());
        q2.pop_front(); ans2.pop_front();
    }

    for(int i=0; i<1024; i++)
    {
        ASSERT_TRUE(*q.front() == ans.front());
        ASSERT_TRUE(*q.back() == ans.back());
        q.pop_back(); ans.pop_back();
    }

    ASSERT_TRUE(q.empty());
    ASSERT_TRUE(q2.empty());
}

TEST(SchedulerNeeds, AWholeTest) {
    const size_t len = 10240;
    Deque<int> q;
    std::deque<int> ans;

    for(size_t i=0; i<len; i++)
    {
        int op = rand() % 5;
        if(op == 0 || op == 1)             // push_back new element.
        {
            int val = rand();
            q.push_back(val);
            ans.push_back(val);
        }
        else if(op == 2 || op == 3)        // pop_front an element.
        {
            ASSERT_TRUE(q.size() == ans.size());
            if(!q.empty())
            {
                ASSERT_TRUE(*q.front()==ans.front());
                q.pop_front();
                ans.pop_front();
            }
        }
        else                    // clear deque
        {
            q.clear();
            ans.clear();
        }

        ASSERT_TRUE(q.size()==ans.size());
        for(size_t j=0; j<q.size(); j++)
            ASSERT_TRUE(q[j]==ans[j]);
    }
}

TEST(MessyPushPop, AWholeTest) {
    const size_t len = 10240;
    Deque<int> q;
    std::deque<int> ans;

    for(size_t i=0; i<len; i++)
    {
        int op = rand() % 4;
        if(op == 0)             // push_back new element.
        {
            int val = rand();
            q.push_back(val);
            ans.push_back(val);
        }
        else if(op == 1)        // push_front
        {
            int val = rand();
            q.push_front(val);
            ans.push_front(val);
        }
        else if(op == 2)        // pop_front an element.
        {
            ASSERT_TRUE(q.size() == ans.size());
            if(!q.empty())
            {
                ASSERT_TRUE(*q.front()==ans.front());
                q.pop_front();
                ans.pop_front();
            }
        }
        else if(op == 3)        // pop_back an element.
        {
            ASSERT_TRUE(q.size() == ans.size());
            if(!q.empty())
            {
                ASSERT_TRUE(*q.back()==ans.back());
                q.pop_back();
                ans.pop_back();
            }
        }

        ASSERT_TRUE(q.size()==ans.size());
        for(size_t j=0; j<q.size(); j++)
            ASSERT_TRUE(q[j]==ans[j]);
    }
}


#endif
