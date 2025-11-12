# Синхронизация в Windows SDK (C++)

## 1. Мьютекс (Mutex)
**Назначение**:  
Взаимное исключение для потоков **внутри одного или разных процессов**. Обеспечивает эксклюзивный доступ к ресурсу.  

**Особенности**:
- Объект ядра Windows (ядерный объект).
- Поддерживает именованные мьютексы для межпроцессного взаимодействия.
- Медленнее критических секций из-за переходов в ядро.

**Ключевые функции**:
```cpp
HANDLE CreateMutex(
  LPSECURITY_ATTRIBUTES lpMutexAttributes,
  BOOL bInitialOwner,
  LPCWSTR lpName
);

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
BOOL ReleaseMutex(HANDLE hMutex);
CloseHandle(HANDLE hObject);
```

**Пример**:
```cpp
HANDLE mutex = CreateMutex(NULL, FALSE, L"MyMutex");
if (WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0) {
    // Работа с общим ресурсом
    ReleaseMutex(mutex);
}
CloseHandle(mutex);
```

---

## 2. Критическая секция (Critical Section)
**Назначение**:  
Синхронизация потоков **только внутри одного процесса**.  

**Особенности**:
- Не является объектом ядра (работает в пользовательском режиме).
- Быстрее мьютексов при отсутствии конкуренции.
- Не поддерживает межпроцессную синхронизацию.

**Ключевые функции**:
```cpp
void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
```

**Пример**:
```cpp
CRITICAL_SECTION cs;
InitializeCriticalSection(&cs);

EnterCriticalSection(&cs);
// Работа с общим ресурсом
LeaveCriticalSection(&cs);

DeleteCriticalSection(&cs);
```

---

## 3. Семафор (Semaphore)
**Назначение**:  
Ограничение количества потоков, одновременно обращающихся к ресурсу (например, пул соединений).  

**Особенности**:
- Объект ядра Windows.
- Использует счетчик для контроля доступа.
- Может работать между процессами.

**Ключевые функции**:
```cpp
HANDLE CreateSemaphore(
  LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
  LONG lInitialCount,
  LONG lMaximumCount,
  LPCWSTR lpName
);

BOOL ReleaseSemaphore(
  HANDLE hSemaphore,
  LONG lReleaseCount,
  LPLONG lpPreviousCount
);
```

**Пример** (максимум 3 одновременных доступа):
```cpp
HANDLE sem = CreateSemaphore(NULL, 3, 3, NULL);
if (WaitForSingleObject(sem, INFINITE) == WAIT_OBJECT_0) {
    // Работа с ресурсом
    ReleaseSemaphore(sem, 1, NULL);
}
CloseHandle(sem);
```

---

## Сравнение
| Механизм          | Межпроцессный | Скорость      | Особенности                     |
|--------------------|---------------|---------------|----------------------------------|
| **Мьютекс**        | ✅            | Медленный     | Рекурсивный захват              |
| **Критическая секция** | ❌         | Очень быстрый | Только внутри процесса          |
| **Семафор**        | ✅            | Средняя       | Контроль количества потоков     |

> **Важно**:  
> - Всегда освобождайте синхронизирующие объекты (например, `CloseHandle`, `DeleteCriticalSection`).  
> - Избегайте длительных операций внутри блокировок.  
> - Для современного C++ предпочтительнее использовать `std::mutex`, `std::lock_guard` и т.д., но Windows-специфичные объекты необходимы для низкоуровневой работы.

# CRITICAL_SECTION vs MUTEX: ключевые различия

## CRITICAL_SECTION
```cpp
CRITICAL_SECTION cs;
InitializeCriticalSection(&cs);
EnterCriticalSection(&cs);
// Критический код
LeaveCriticalSection(&cs);
DeleteCriticalSection(&cs);
```

**Особенности**:
- ✅ Работает **только внутри одного процесса**
- ✅ Очень быстрая (реализована в пользовательском режиме)
- ❌ Не поддерживает таймауты
- ❌ При аварийном завершении потока — deadlock
- ❌ Не имеет имени, нельзя использовать между процессами

**Когда использовать**:
- Синхронизация потоков в одном приложении
- Высокочастотные операции (например, обработка сетевых пакетов)
- Когда не нужна межпроцессная синхронизация

## MUTEX (именованный)
```cpp
HANDLE mutex = CreateMutex(NULL, FALSE, L"MyGlobalMutex");
WaitForSingleObject(mutex, INFINITE);
// Критический код
ReleaseMutex(mutex);
CloseHandle(mutex);
```

**Особенности**:
- ✅ Работает **между разными процессами**
- ✅ Поддерживает таймауты (`WaitForSingleObject(mutex, 5000)`)
- ✅ При аварийном завершении процесса — автоматически освобождается (с состоянием `WAIT_ABANDONED`)
- ❌ Медленнее из-за переходов в ядро Windows
- ❌ Требует явного освобождения через `CloseHandle`

**Когда использовать**:
- Синхронизация между разными экземплярами приложения
- Защита общих ресурсов: файлы, глобальная память, устройства
- Когда нужна гарантия освобождения при падении процесса
- Когда требуется контроль времени ожидания

