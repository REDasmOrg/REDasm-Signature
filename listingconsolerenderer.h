#ifndef LISTINGCONSOLERENDERER_H
#define LISTINGCONSOLERENDERER_H

#include <redasm/disassembler/listing/listingrenderer.h>

class ListingConsoleRenderer : public REDasm::ListingRenderer
{
    public:
        ListingConsoleRenderer(REDasm::DisassemblerAPI* disassembler);
        void renderAll();

    protected:
        virtual void renderLine(const REDasm::RendererLine& rl);
};

#endif // LISTINGCONSOLERENDERER_H
