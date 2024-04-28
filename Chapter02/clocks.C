#include <math.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <future>
#include <mutex>

using std::cout;
using std::endl;

double duration(timespec a, timespec b) {
    return a.tv_sec - b.tv_sec + 1e-9*(a.tv_nsec - b.tv_nsec);
}

constexpr double X = 1e6;

int main() {
    // Busy time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
        double s = 0;
        for (double x = 0; x < X; x += 0.1) s += sin(x);
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);
        cout << "Real time: " << duration(rt1, rt0) << "s, "
                "CPU time: " << duration(ct1, ct0) << "s, "
                "Thread time: " << duration(tt1, tt0) << "s, "
                " result " << s << endl;
    }

    // Idle time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
        sleep(1);
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);
        cout << "Real time: " << duration(rt1, rt0) << "s, "
                "CPU time: " << duration(ct1, ct0) << "s, "
                "Thread time: " << duration(tt1, tt0) << "s" << endl;
    }


    // Thread time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
        double s = 0;
        auto f = std::async(std::launch::async, [&]{ for (double x = 0; x < X; x += 0.1) s += sin(x); });
        f.wait();
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);
        cout << "Real time: " << duration(rt1, rt0) << "s, "
                "CPU time: " << duration(ct1, ct0) << "s, "
                "Thread time: " << duration(tt1, tt0) << "s, "
                " result " << s << endl;
    }

    // Thread time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
        double s1 = 0, s2 = 0;
        auto f1 = std::async(std::launch::async, [&]{ for (double x = 0; x < X; x += 0.1) s1 += sin(x); });
        //auto f2 = std::async(std::launch::async, [&]{ for (double x = 0; x < X; x += 0.1) s2 += sin(x); });
        for (double x = 0; x < X; x += 0.1) s2 += sin(x);
        f1.wait();
        //f2.wait();
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);
        cout << "Real time: " << duration(rt1, rt0) << "s, "
                "CPU time: " << duration(ct1, ct0) << "s, "
                "Thread time: " << duration(tt1, tt0) << "s, "
                " result " << s1 + s2 << endl;
    }

    // Mutex time
    {
        timespec rt0, ct0, tt0;
        clock_gettime(CLOCK_REALTIME, &rt0);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
        double s = 0;
        std::mutex m;
        using guard_t = std::lock_guard<std::mutex>;
        auto f1 = std::async(std::launch::async, [&]{ for (double x = 0; x < X; x += 0.1) { guard_t g(m); s += sin(x); } });
        //auto f2 = std::async(std::launch::async, [&]{ for (double x = 0; x < X; x += 0.1) { guard_t g(m); s += sin(x); } });
        for (double x = 0; x < X; x += 0.1) { guard_t g(m); s += sin(x); };
        f1.wait();
        //f2.wait();
        timespec rt1, ct1, tt1;
        clock_gettime(CLOCK_REALTIME, &rt1);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);
        cout << "Real time: " << duration(rt1, rt0) << "s, "
                "CPU time: " << duration(ct1, ct0) << "s, "
                "Thread time: " << duration(tt1, tt0) << "s, "
                " result " << s << endl;
    }
}
