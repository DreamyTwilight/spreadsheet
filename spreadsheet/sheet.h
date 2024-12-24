#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> sheet_; // vector[col][row]
    std::unordered_set<Cell*> disabling_cache_;
    std::unordered_set<Cell*> hold_cells_ptr_;

    bool IsValidPos(Position pos) const;
    void PrintSheet(std::ostream& output, bool is_print_value) const;
    void IncreasePrintArea(const Position& pos);
    void InsertEmptySell(const Position& pos);
    void InsertPtrCellToUpReferencesListsOfCells(const std::unique_ptr<Cell>& tmp_cell);
    void DisablingTheCache(Cell* cell_ptr);
    void DellUpReference(Position& pos);
    void AddUpReference(Position& pos_modify, const Position& pos_for_add);
    bool IsNewTextCellEqualOldTextCell(Position pos, std::string text);
};