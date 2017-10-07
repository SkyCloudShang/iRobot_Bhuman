/**
 * @file Platform/BHAssert.h
 * This file contains macros for low level debugging
 * @author Thomas Röfer
 * @author Colin Graf
 */

#pragma once

#undef ASSERT
#undef FAIL
#undef VERIFY
#undef TRACE

#include <string>
#include <sstream>

/**
 * Some tools for low level debugging
 */
class Assert
{
public:
#ifndef NDEBUG
  /**
   * Prints a formated message to stdout
   * @param file The name of the current file (__FILE__)
   * @param line The current file line (__LINE__)
   * @param format The format of the message to be printed
   */
  static void print(const char* file, int line, const char* format, ...);
  static void print(const char* file, int line, const std::string& message);

  /**
   * Aborts the execution of the program
   */
  static void abort();
#endif // NDEBUG

#ifdef TARGET_ROBOT
  /**
   * Initializes some log message ring buffers associated to the current thread
   * @param name The name of the current thread
   * @return Whether the initialization was successful
   */
  static bool logInit(const char* name);

  /**
   * Adds a message to the log message ring buffer
   * @param track The id of the used ring buffer
   * @param file The name of the current file (__FILE__)
   * @param line The current file line (__LINE__)
   * @param message The message to be added
   */
  static void logAdd(int track, const char* file, int line, const std::string& message);

  /**
   * Dumps the content of the log message ring buffers to stderr or /home/nao/logs
   * @param toStderr Whether to dump the content to stderr or /home/nao/logs
   * @param termSignal A signal that was emitted to terminate the bhuman process
   */
  static void logDump(int termSignal);
#endif // TARGET_ROBOT
};

/**
 * ASSERT prints a message if \c cond is \c false and NDEBUG is not defined.
 * ASSERT does not evaluate \c cond if NDEBUG is defined.
 * @param c The condition to be checked.
 */
#ifdef NDEBUG
#define ASSERT(cond) ((void)0)
#elif defined TARGET_ROBOT
#define ASSERT(cond) ((void)((cond) ? 0 : (Assert::logAdd(0, __FILE__, __LINE__, "ASSERT(" #cond ") failed"), Assert::abort(), 0)))
#elif defined WINDOWS
#define ASSERT(cond) ((void)((cond) ? 0 : (Assert::abort(), 0)))
#else
#define ASSERT(cond) ((void)((cond) ? 0 : (Assert::print(__FILE__, __LINE__, "ASSERT(%s) failed", #cond), Assert::abort(), 0)))
#endif

/**
 * ASSERTs cond
 * if NDEBUG is defined the safe value will be returned in the case of a failing conditional check
 * it is recommended to use SAFE_ASSERT just for cond that are fast evaluated
 */
#ifdef NDEBUG
#define SAFE_ASSERT(cond, safe) while(!(cond)) return (safe)
#else
#define SAFE_ASSERT(cond, safe) ASSERT(cond)
#endif

/**
 * FAIL is equivalent to ASSERT(false) and additionally prints a text.
 * This text is passed into a std::strigstream, thus FAIL("error " << 1) is a valid expression.
 */
#ifdef NDEBUG
#define FAIL(text) ((void)0)
#else
#if defined TARGET_ROBOT
#define _FAIL_OUTPUT Assert::logAdd(0, __FILE__, __LINE__, sstream.str());
#else
#define _FAIL_OUTPUT Assert::print(__FILE__, __LINE__, sstream.str());
#endif
#define FAIL(text) \
  do \
  { \
    std::stringstream sstream; \
    sstream << "FAIL: " << text; \
    _FAIL_OUTPUT \
    Assert::abort(); \
  } \
  while(false)
#endif

/**
 * VERIFY prints a message if \cond cond is \c false and NDEBUG is not defined.
 * VERIFY does evaluate \c cond even if NDEBUG is defined.
 * @param c The condition to be checked.
 */
#ifdef NDEBUG
#define VERIFY(cond) ((void)(cond))
#elif defined WINDOWS
#define VERIFY(cond) ((void)((cond) ? 0 : (Assert::abort(), 0)))
#elif defined TARGET_ROBOT
#define VERIFY(cond) ((void)((cond) ? 0 : (Assert::logAdd(0, __FILE__, __LINE__, "VERIFY(" #cond ") failed"), Assert::abort(), 0)))
#else
#define VERIFY(cond) ((void)((cond) ? 0 : (Assert::print(__FILE__, __LINE__, "VERIFY(%s) failed", #cond), Assert::abort(), 0)))
#endif

/**
 * TRACE prints a message if NDEBUG is not defined.
 */
#ifdef NDEBUG
#define TRACE(...) ((void)0)
#else
#define TRACE(...) Assert::print(__FILE__, __LINE__, __VA_ARGS__)
#endif

/**
 * Initializes some log message ring buffers associated to the current thread
 * @param name The name of the current thread
 * @return Whether the initialization was successful
 */
#if defined NDEBUG || !defined TARGET_ROBOT
#define BH_TRACE_INIT(name) ((void)0)
#else
#define BH_TRACE_INIT(name) VERIFY(Assert::logInit(name))
#endif

/**
 * Adds a message to the log message ring buffer
 * @param message The message to be added
 */
#if defined NDEBUG || !defined TARGET_ROBOT
#define BH_TRACE_MSG(message) ((void)0)
#else
#define BH_TRACE_MSG(message) Assert::logAdd(0, __FILE__, __LINE__, message)
#endif

/**
 * Adds the current line of code to the log message ring buffer
 */
#if defined NDEBUG || !defined TARGET_ROBOT
#define BH_TRACE ((void)0)
#else
#define BH_TRACE Assert::logAdd(0, __FILE__, __LINE__, "")
#endif
