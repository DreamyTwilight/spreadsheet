#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <unordered_set>

using namespace std::literals;

Sheet::~Sheet() {}
void Sheet::IncreasePrintArea(const Position& pos) {
    while (sheet_.empty() || pos.col > int(sheet_.size()) - 1) {
        sheet_.emplace_back();
    }
    while (sheet_[pos.col].empty() || pos.row > int(sheet_[pos.col].size()) - 1) {
        sheet_[pos.col].emplace_back(nullptr);
    }
}

void Sheet::InsertEmptySell(const Position& pos) {
    std::unique_ptr<Cell> empty_cell = std::make_unique<Cell>(*this);
    empty_cell->Set(""s);
    sheet_[pos.col][pos.row].swap(empty_cell);
}

void Sheet::InsertPtrCellToUpReferencesListsOfCells(const std::unique_ptr<Cell>& tmp_cell) {
    for (const auto& cell_position : tmp_cell->GetReferencedCells()) {
        IncreasePrintArea(cell_position);
        if (GetConcreteCell(cell_position) == nullptr) {
            InsertEmptySell(cell_position);
        }
        sheet_[cell_position.col][cell_position.row]->InsertCellPtrToUpReferencedList(tmp_cell.get());
    }
}

void Sheet::DisablingTheCache(Cell* cell_ptr) {
    if (disabling_cache_.find(cell_ptr) != disabling_cache_.end()) {
        for (Cell* cell : cell_ptr->GetUpReferenceCells()) {
            cell->ClearCache();
        }
        disabling_cache_.insert(cell_ptr);
        for (Cell* cell : cell_ptr->GetUpReferenceCells()) {
            if (cell->GetUpReferenceCells().empty()) {
                disabling_cache_.insert(cell);
            }
            else {
                DisablingTheCache(cell);
            }
        }
    }
}

void Sheet::DellUpReference(Position& pos) {
    Cell* cell_for_dell = sheet_[pos.col][pos.row].get();
    for (Position& pos_modify : cell_for_dell->GetReferencedCells()) {
        Cell* cell_modify = sheet_[pos_modify.col][pos_modify.row].get();
        if (cell_modify->GetUpReferenceCells().count(cell_for_dell) > 0) {
            cell_modify->GetUpReferenceCells().erase(cell_for_dell);
        }
        if (cell_modify->GetUpReferenceCells().empty() && cell_modify->GetText() == ""s) {
            ClearCell(pos_modify);
        }
    }
}

static void CheckValidPositionInTable(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
}

bool Sheet::IsNewTextCellEqualOldTextCell(Position pos, std::string text) {
    std::string text_in_pos = ""s;
    if (IsValidPos(pos)) {
        if (sheet_[pos.col][pos.row].get() != nullptr) {
            text_in_pos = GetCell(pos)->GetText();
        }
    }
    return text_in_pos == text;
}

void Sheet::SetCell(Position pos, std::string text) {
    CheckValidPositionInTable(pos);
    if (text.find(pos.ToString()) != text.npos) {
        throw CircularDependencyException(""s);
    }
    if (!IsNewTextCellEqualOldTextCell(pos, text)) {
        std::unique_ptr<Cell> tmp_cell = std::make_unique<Cell>(*this);
        tmp_cell->Set(text);
        IncreasePrintArea(pos);
        if (sheet_[pos.col][pos.row] != nullptr) {
            if (sheet_[pos.col][pos.row]->IsUpReferenced()) {
                tmp_cell->CopyUpReferenceFromCell(*sheet_[pos.col][pos.row]);
            }
            ClearCell(pos);
            IncreasePrintArea(pos);
        }
        sheet_[pos.col][pos.row] = std::move(tmp_cell);
        if (sheet_[pos.col][pos.row]->IsReferenced()) {
            InsertPtrCellToUpReferencesListsOfCells(sheet_[pos.col][pos.row]);
        }
    }
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    return sheet_[pos.col][pos.row].get();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    return sheet_[pos.col][pos.row].get();
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return IsValidPos(pos) ? &(*sheet_[pos.col][pos.row]) : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckValidPositionInTable(pos);
    return IsValidPos(pos) ? &(*sheet_[pos.col][pos.row]) : nullptr;
}
 
void Sheet::ClearCell(Position pos) {
    CheckValidPositionInTable(pos);
    if (IsValidPos(pos)) {
        if (sheet_[pos.col][pos.row]->IsReferenced()) {
            DellUpReference(pos);
        }
        if (sheet_[pos.col][pos.row]->IsUpReferenced()) {
            disabling_cache_.clear();
            sheet_[pos.col][pos.row]->ClearCache();
            DisablingTheCache(sheet_[pos.col][pos.row].get());
            std::unique_ptr<Cell> tmp_cell = std::make_unique<Cell>(*this);
            tmp_cell->Set(""s);
            tmp_cell->CopyUpReferenceFromCell(*sheet_[pos.col][pos.row]);
        }
        else {
            sheet_[pos.col][pos.row] = nullptr;
            while (!sheet_[pos.col].empty() && sheet_[pos.col][int(sheet_[pos.col].size()) - 1] == nullptr) {
                sheet_[pos.col].pop_back();
            }
            while (!sheet_.empty() && sheet_[int(sheet_.size()) - 1].empty()) {
                sheet_.pop_back();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    int cell_max = 0;
    for (const auto& col : sheet_) {
        if (cell_max < int(col.size())) {
            cell_max = int(col.size());
        }
    }
    return { cell_max, int(sheet_.size()) };
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintSheet(output, true);
}

void Sheet::PrintTexts(std::ostream& output) const {
    PrintSheet(output, false);
}

// is pos in print area?
bool Sheet::IsValidPos(Position pos) const {
    return size_t(pos.col) < sheet_.size() && pos.row < int(sheet_[pos.col].size());
}

std::ostream& operator <<(std::ostream& os, const CellInterface::Value& value) {
    std::visit([&os](auto&& agr) {
        os << agr;
        }, value
    );
    return os;
}

void Sheet::PrintSheet(std::ostream& output, bool is_print_value) const {
    const auto [rows, cols] = GetPrintableSize();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (int(sheet_[col].size()) > row) {
                if (sheet_[col].empty() || sheet_[col][row] == nullptr) {
                    if (col < cols - 1) {
                        output << '\t';
                    }
                }
                else {
                    if (is_print_value) {
                        output << sheet_[col][row]->GetValue();
                    }
                    else {
                        output << sheet_[col][row]->GetText();
                    }
                    if (col < cols - 1) {
                        output << '\t';
                    }
                }
            }
            else {
                if (col < cols - 1) {
                    output << '\t';
                }
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}