#include <iostream>
#include <fstream>
#include <map>

using namespace std;

/// <summary>
/// Класс для константных таблиц. Является обёрткой над map\string, int\, использует string-строки в качестве
/// ключа для поиска и int-значение в качестве номера этого ключа в линейной таблице
/// </summary>
class ConstTable {
public:
   map<string, int> data;

   /// <summary>
   /// Функция поиска элемента в таблице по ключу (названию элемента)
   /// </summary>
   /// <param name="elem"> - название элемента</param>
   /// <returns>-1, если элемента в таблице нет, иначе возвращает номер элемента в таблице</returns>
   int find(const string& elem) {
      // Пробуем найти элемент в таблице
      auto elemPtr = data.find(elem);

      // Если элемент не найден
      if (elemPtr == data.end()) {
         return -1;
      } else {
         // Возвращаем второй элемент (int) из пары значений в таблице
         return elemPtr->second;
      }
   }

   /// <summary>
   /// Метод чтения данных таблицы из файла
   /// </summary>
   /// <param name="filePath"> - путь до файла</param>
   void readFromFile(string filePath) {
      auto file = ifstream(filePath);
      if (!file.is_open()) {
         throw runtime_error("Cannot open file " + filePath);
      }

      while (!file.eof()) {
         int num;
         string str;
         // Считываем построчно пары (число строка)
         file >> num >> str;

         // Добавляем их в хэш-таблицу
         auto pair = std::pair(str, num);
         data.insert(pair);
      }
   }
};

enum class Type {
   undefined,
   integer,
};

struct MetaData {
   Type type = Type::undefined;
   int value = 0;
};

struct ConstMetaData {
   Type type = Type::integer;
};

/// <summary>
/// Класс для переменных таблиц. Является обёрткой над map\string,pair(int, MetaData)\, использует string-строки в качестве ключа
/// для поиска, int значение для определения номера ключа в таблице и MetaData для получения значения переменной
/// </summary>
template<typename T>
class VariableTable {
private:
   // Счётчик числа переменных
   int counter = 0;

public:
   map<string, pair<int, T>> data;

   /// <summary>
   /// Функция поиска элемента по ключу в таблице
   /// </summary>
   /// <param name="elem"> - ключ элемента в таблице</param>
   /// <returns>позиция элемента в таблице (по полю [int])</returns>
   int find(const string& elem) {
      // Пробуем найти элемент в таблице
      auto elemPtr = data.find(elem);

      // Если элемент не найден
      if (elemPtr == data.end()) {
         return -1;
      } else {
         // Внебрачный сын int и MetaData
         pair<int, T> inita = elemPtr->second;
         return inita.first;
      }
   }

   T* findMetaByIndex(int index) {
      // Если индекс элемента больше, чем в таблице есть
      if (index >= counter || index < 0) {
         return nullptr;
      }

      T* metaPtr = nullptr;
      for (auto& pair : data) {
         auto& value = pair.second;       // pair<int, T>
         if (value.first == index) {
            metaPtr = &(value.second);
            break;
         }
      }

      return metaPtr;
   }

   shared_ptr<pair<string, T&>> findByIndex(int index) {
      if (index >= counter || index < 0) {
         return nullptr;
      }

      for (auto& pair : data) {
         auto& value = pair.second;
         if (value.first == index) {
            std::pair<string, T&> elem = {pair.first, value.second};
            return make_shared<std::pair<string, T&>>(elem);
         }
      }
   }

   /// <summary>
   /// Добавляет элемент в таблицу, либо возвращает номер существующего элемента
   /// </summary>
   /// <param name="key"> - ключ элемента</param>
   /// <param name="metadata"> - метаданные элемента</param>
   /// <returns>номер вставленного или уже существующего в таблице элемента</returns>
   int add(string key, T metadata = T()) {
      int pos = this->find(key);

      // Если элемент с таким ключом уже существует
      if (pos >= 0) {
         // Обновляем метаданные существующего элемента
         data.at(key).second = metadata;
         return pos;
      }

      // формируем новую пару для хэш-таблицы и вставляем её в таблицу
      auto pair = std::pair(key, std::pair(counter, metadata));
      data.insert(pair);
      return counter++;
   }
};

void constTablesTests() {
   cout << "Начало тестирования константных таблиц:\n";
   auto constTable = ConstTable();

   cout << "  - Тестирование на считывание таблицы из несуществующего файла\n";
   cout << "    Программа должна выдать соответствующее исключение\n";
   try {
      constTable.readFromFile("");
      cout << "  Метод [readFromFile] должен был выдать исключение\n\n";
   } catch (runtime_error e) {
      cout << "  Вызвано исключение: " << e.what() << ", тест пройден\n\n";
   }

   cout << "  - Тестирование на считывание таблицы из существующего файла\n";
   cout << "    Программа должна корректно считать данные из таблицы и не выдавать исключений\n";
   try {
      constTable.readFromFile("const_tables/keywords.txt");
      cout << "  Метод успешно считал данные из файла. Тест пройден\n\n";
   } catch (runtime_error e) {
      cout << "  Вызвано исключение: " << e.what() << ", тест не пройден\n\n";
      // Если вышло исключение считывания из файла, то дальнейшие тесты на работу с таблицей не имеют смысла
      return;
   }

   cout << "  - Тестирование на поиск существующего элемента [main] в таблице.\n";
   cout << "    Таблица должна вернуть его положение в таблице: [1]\n";
   auto indexOfMain = constTable.find("main");
   if (indexOfMain == -1) {
      cout << "  Элемент [main] не был найден в таблице. Тест провален\n\n";
   } else {
      cout << "  Индекс элемента [main] в таблице: " << indexOfMain << endl;
      if (indexOfMain != 1) {
         cout << "  Найденный индекс не соответствует индексу в таблице. Тест провален\n\n";
      } else {
         cout << "  Найденный индекс соответствует индексу в таблице. Тест пройден\n\n";
      }
   }

   cout << "  - Тестирование на поиск в таблице несуществующего элемента [MaIN].\n";
   cout << "    Таблица должна вернуть индекс == [-1]\n";
   auto indexOfMaIN = constTable.find("MaIN");
   if (indexOfMaIN != -1) {
      cout << "  Элемент MaIN был найден в таблице по индексу " << indexOfMaIN << ". Тест провален\n\n";
   } else {
      cout << "  Элемент MaIN не был найден в таблице. Тест пройден\n\n";
   }
   cout << "Конец тестов для константных таблиц\n\n ****************************** \n\n";
}

void variableTablesTests() {
   cout << "Начало тестирования таблиц идентификаторов:\n";
   auto identifiersTable = VariableTable<MetaData>();

   cout << "  - Тестирование на поиск элемента в пустой таблице.\n";
   cout << "    Таблица должна вернуть индекс [-1]\n";
   auto indexOfElemInEmptyTable = identifiersTable.find("sum");
   if (indexOfElemInEmptyTable != -1) {
      cout << "  Таблица вернула индекс элемента равный [" << indexOfElemInEmptyTable << "]. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс элемента равный [-1]. Тест пройден\n\n";
   }

   cout << "  - Тестирование на добавление нового элемента в пустую таблицу.\n";
   cout << "    Таблица должна вернуть индекс добавленного элемента, равный [0]\n";
   auto indexOfFirstElem = identifiersTable.add("sum");
   if (indexOfFirstElem != 0) {
      cout << "  Таблица вернула индекс нового элемента, не равный 0. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс нового элемента, равный 0. Тест пройден\n\n";
   }

   cout << "  - Тест на поиск существующего единственного элемента в таблице.\n";
   cout << "    Таблица должна вернуть индекс [0] этого элемента\n";
   indexOfFirstElem = identifiersTable.find("sum");
   if (indexOfFirstElem != 0) {
      cout << "  Таблица не смогла найти существующий элемент либо вернула неверный индекс элемента. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс существующего элемента, равный 0. Тест пройден\n\n";
   }

   cout << "  - Тест на поиск несуществующего элемента в непустой таблице.\n";
   cout << "    Таблица должна вернуть индекс [-1]\n";
   auto indexOfElemInNonEmptyTable = identifiersTable.find("mult");
   if (indexOfElemInNonEmptyTable != -1) {
      cout << "  Таблица вернула индекс несуществующего элемента, равный [" << indexOfElemInNonEmptyTable << "]. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс [-1]. Тест пройден\n\n";
   }

   cout << "  - Тест на добавление в непустую таблицу нового элемента.\n";
   cout << "    Таблица должна вернуть индекс нового элемента, равный [1]\n";
   auto indexOfSecondElem = identifiersTable.add("mult");
   if (indexOfSecondElem != 1) {
      cout << "  Таблица вернула индекс нового элемента [" << indexOfSecondElem << "]. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс нового элемента [1]. Тест пройден\n\n";
   }

   cout << "  - Тест на поиск существующего элемента в таблице из двух элементов.\n";
   cout << "    Таблица должна вернуть индекс [0] этого элемента\n";
   indexOfFirstElem = identifiersTable.find("sum");
   if (indexOfFirstElem != 0) {
      cout << "  Таблица не смогла найти существующий элемент либо вернула неверный индекс элемента. Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс существующего элемента, равный 0. Тест пройден\n\n";
   }

   cout << "  - Тест на добавление уже существующего элемента в таблицу.\n";
   cout << "    Таблица должна вернуть индекс [1] этого элемента\n";
   auto indexOfNewElem = identifiersTable.add("mult");
   if (indexOfNewElem != 1) {
      cout << "  Таблица вернула индекс какого-то другого элемента [" << indexOfNewElem << "] Тест провален\n\n";
   } else {
      cout << "  Таблица вернула индекс прежнего элемента. Тест пройден\n\n";
   }

   cout << "Конец тестов для таблиц идентификаторов\n\n ****************************** \n\n";
}

void tablesTests() {
   constTablesTests();
   variableTablesTests();
}

int main() {
   setlocale(LC_ALL, "ru-RU.utf-8");

   //tablesTests();
   //auto varTable = VariableTable<MetaData>();

   //varTable.add("x");
   //varTable.add("y");

   //auto xMetaDataPtr = varTable.findMetaByIndex(0);
   //if (xMetaDataPtr) {
   //   cout << "Элемент по индексу 0 найден\n";
   //   xMetaDataPtr->type = Type::integer;

   //} else {
   //   cout << "ОШИБОКА\n";
   //}

   //xMetaDataPtr = varTable.findMetaByIndex(0);
   //if (xMetaDataPtr) {
   //   if (xMetaDataPtr->type == Type::integer) {
   //      cout << "Тип есть integer\n";
   //   }
   //}

   //auto noneMetaDataPtr = varTable.findMetaByIndex(2);
   //if (noneMetaDataPtr) {
   //   cout << "ОШИБОКА\n";
   //} else {
   //   cout << "По индексу 2 ничего не найдено\n";
   //}
}