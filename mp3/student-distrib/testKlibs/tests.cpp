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

#endif
