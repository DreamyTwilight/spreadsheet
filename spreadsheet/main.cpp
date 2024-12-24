#include "common.h"
#include "test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

namespace {

    void TestEmpty() {
        auto sheet = CreateSheet();
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 0, 0 }));
    }

    void TestInvalidPosition() {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell(Position{ -1, 0 }, "");
        }
        catch (const InvalidPositionException&) {
        }
        try {
            sheet->GetCell(Position{ 0, -2 });
        }
        catch (const InvalidPositionException&) {
        }
        try {
            sheet->ClearCell(Position{ Position::MAX_ROWS, 0 });
        }
        catch (const InvalidPositionException&) {
        }
    }

    void TestSetCellPlainText() {
        auto sheet = CreateSheet();

        auto checkCell = [&](Position pos, std::string text) {
            sheet->SetCell(pos, text);
            CellInterface* cell = sheet->GetCell(pos);
            ASSERT(cell != nullptr);
            ASSERT_EQUAL(cell->GetText(), text);
            ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
            };

        checkCell("A1"_pos, "Hello");
        checkCell("A1"_pos, "World");
        checkCell("B2"_pos, "Purr");
        checkCell("A3"_pos, "Meow");

        const SheetInterface& constSheet = *sheet;
        ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

        sheet->SetCell("A3"_pos, "'=escaped");
        CellInterface* cell = sheet->GetCell("A3"_pos);
        ASSERT_EQUAL(cell->GetText(), "'=escaped");
        ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
    }

    void TestClearCell() {
        auto sheet = CreateSheet();

        sheet->SetCell("C2"_pos, "Me gusta");
        sheet->ClearCell("C2"_pos);
        ASSERT(sheet->GetCell("C2"_pos) == nullptr);

        sheet->ClearCell("A1"_pos);
        sheet->ClearCell("J10"_pos);
    }
    void TestPrint() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B2"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=1/0");

        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 2 }));

        std::ostringstream texts;
        sheet->PrintTexts(texts);
        ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

       std::ostringstream values;
       sheet->PrintValues(values);
       ASSERT_EQUAL(values.str(), "#ARITHM!\t\nmeow\t3\n");

        sheet->ClearCell("B2"_pos);
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 1 }));
    }

}  // namespace

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestInvalidPosition);
    RUN_TEST(tr, TestSetCellPlainText);
    RUN_TEST(tr, TestClearCell);
    RUN_TEST(tr, TestPrint);

 //  auto sheet = CreateSheet();
 //  sheet->SetCell("A1"_pos, "=(1+2)*3");
 //  auto sell = sheet->GetCell("A1"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("A2"_pos, "some");
 //  sell = sheet->GetCell("A2"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("B1"_pos, "=1+2*3");
 //  sell = sheet->GetCell("B1"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("B2"_pos, "text");
 //  sell = sheet->GetCell("B2"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("B5"_pos, "=1/0");
 //  sell = sheet->GetCell("B5"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("C2"_pos, "here");
 //  sell = sheet->GetCell("C2"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("C3"_pos, "'and");
 //  sell = sheet->GetCell("C3"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  sheet->SetCell("D3"_pos, "'here");
 //  sell = sheet->GetCell("D3"_pos);
 //  std::cout << sell->GetText() << std::endl;
 //
 //  auto size = sheet->GetPrintableSize();
 //
 //  std::stringstream ss;
 //  
 //  std::cout << "*********************" << std::endl;
 //
 //  sheet->PrintTexts(std::cout);
 //  std::cout << std::endl;
 //  sheet->PrintValues(std::cout);


}