#include "listingconsolerenderer.h"
#include <redasm/plugins/format.h>
#include <iostream>

ListingConsoleRenderer::ListingConsoleRenderer(REDasm::DisassemblerAPI *disassembler): REDasm::ListingRenderer(disassembler) { }
void ListingConsoleRenderer::renderAll() { this->render(0, m_document->size()); }

void ListingConsoleRenderer::renderLine(const REDasm::RendererLine &rl)
{
    REDasm::ListingItem* item = m_document->itemAt(rl.documentindex);

    if(!item->is(REDasm::ListingItem::InstructionItem))
        return;

    REDasm::InstructionPtr instruction = m_document->instruction(item->address);
    REDasm::BufferRef buffer = m_disassembler->format()->buffer(instruction->address);
    std::cout << rl.text << "\t; " << REDasm::hexstring(buffer, instruction->size) << std::endl;
}
