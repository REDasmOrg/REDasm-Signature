#include "listingconsolerenderer.h"
#include <redasm/plugins/loader.h>
#include <iostream>

ListingConsoleRenderer::ListingConsoleRenderer(REDasm::DisassemblerAPI *disassembler): REDasm::ListingRenderer(disassembler) { }
void ListingConsoleRenderer::renderAll() { this->render(0, m_document->size()); }

void ListingConsoleRenderer::renderLine(const REDasm::RendererLine &rl)
{
    REDasm::ListingItem* item = m_document->itemAt(rl.documentindex);

    if(!item->is(REDasm::ListingItem::InstructionItem))
        return;

    REDasm::InstructionPtr instruction = m_document->instruction(item->address);
    REDasm::BufferView view = m_disassembler->loader()->view(instruction->address);
    std::cout << rl.text << "\t; " << REDasm::hexstring(view, instruction->size) << std::endl;
}
