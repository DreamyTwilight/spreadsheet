# spreadsheet - Электронная таблица
### Системые требования:
  1. C++17(STL)
  2. GCC (MinG w64) 11.2.0  
<br>
Программа реализует функциональность электронной таблицы, подобной Excel или Google Sheets в сильно упрощенном виде. 

## Сборка проекта при помощи Cmake 

1. Копируем себе папку проекта **`spreadsheet`** с репоозитория.
2. Создаем в ней пару папки **`antlr4_runtime`** и **`build`**, создав следующее дерево папок:

**`spreadsheet`**/<br>
├── **`antlr4_runtime`**/<br>
├── **`build`**/<br>

3. В папку **`antlr4_runtime`** помещаем файлы из архива **`antlr4_runtime.zip`**, размещенного в данном репозитории.
4. В консоли переходим в папку **`spreadsheet`** и выполняем команду:<br>
   **`antlr4 -Dlanguage=Cpp Formula.g4`**<br>
Получаем новые файлы:<br>
**`Formula.interp`**, **`Formula.tokens`**  — вспомогательные текстовые файлы для вашего удобства;<br>
**`FormulaLexer.{cpp,h}`** — код лексического анализатора;<br>
**`FormulaParser.{cpp,h}`** — код синтаксического анализатора;<br>
**`FormulaListener.{cpp,h}`**, **`FormulaBaseListener.{cpp,h}`** — код listener'а, разновидности паттерна visitor для дерева разбора. Он позволит обходить дерево разбора и строить абстрактное синтаксическое дерево для вычисления формул.<br>
5. Переходим в консоли в папку **`build`**<br>
6. Выполняем команду **`cmake ..`**<br>
7. Ищем файлы **`CMakeCache.txt`** и все удаляем<br>
8. Выполняем команду **`cmake ../ -G "Visual Studio 17 2022`**<br>
9. Открываем **`spreadsheet.sln`** в MS VS 2022, удаляем в папках **`Header Files`** и **`Source Files`** проекта задвоения .h и .cpp файлов.<br>
11. В MS VS 2022 нажимаем F7 (Build Solution) и ждем окончания сборки проекта.<br>

###Стек технологий
1. **C++17**:
2. **STL**:
3. **ANTLR**:
4. **CMake**:
5. **Unit тестирование**:

### Работа программы
Программа создает пустое пространство для электронной таблицы с максимально возможными размерами поля, определяемыми константами MAX_ROWS и MAX_COLS.<br>
Пользователь может вводить текст или формулы с помощью метода `SetCell`. Если текст начинается с `=`, он интерпретируется как формула, и программа запускает процесс её анализа и вычисления.<br>
Реализован функционал контроля корректности ввода и вычисления формулы, а так же запрет ввода формул, приводящих к зацикливанию. <br>
Вычисленное значение формулы кэшируется. При очищении или изменении в ячейках, производится анализ зависимостей с другими ячейками таблицы, при необходимости зависимости корректируются. Кэш формульных ячеек, использующих данные изменяемой ячейки, инвалидируется. <br>
Программа позволяет выводить содержимое таблицы как в виде текстов (метод `PrintTexts`), так и в виде вычисленных значений (метод `PrintValues`). Размер выводимого поля вычисляется автоматически, исходя из адресации введенных ячеек.
В программе не реализован UI, работоспособность иллюстрируется тестами.<br>

### Архитектура программы

   - **`Cell`**: класс для представления ячейки в таблице. Ячейка может быть пустой, содержать текст или формулу. Для хранения разных типов состояния используется паттерн "Состояние" (State Pattern) через вложенные классы.
     
   - **`Sheet`**: класс управляет набором ячеек, организованным в виде двумерного массива. Реализован интерфейс `SheetInterface`, предоставляющий методы для установки значений в ячейки, получения их значений или текстов, а также очистки ячеек и печати информации о таблице. Ячейки хранятся в виде векторов, что позволяет динамически изменять размер таблицы при добавлении новых строк или столбцов.
     
   - **`Formula`**: класс вычисления значений на основе переданных аргументов (значений других ячеек). Реализован интерфейс `FormulaInterface`. Обрабатываются случаи, когда формулы генерируют ошибки, такие как деление на ноль или неправильные ссылки на ячейки.
     
   - **`Position`**: структура, представляющая положение ячейки в таблице (строка и столбец). Содержит методы для проверки корректности позиции и преобразования ее в строку и обратно.

   - Программа определяет различные исключения, чтобы обрабатывать ошибки при работе с ячейками, такие как `InvalidPositionException`, `FormulaException`, и `CircularDependencyException`. Эти исключения позволяют программе безопасно реагировать на неправильные операции, сохраняя консистентность данных.

   - **`TestRunner`**: класс системы тестирования. Она позволяет выполнять автоматические тесты, проверяющие корректность работы различных компонентов программы, таких как установка значений, очистка ячеек и обработка формул.<br>
