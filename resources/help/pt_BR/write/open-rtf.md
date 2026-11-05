# Abrir arquivos RTF
<!-- keywords: rtf, texto formatado, rich text format, abrir rtf, .rtf, documentos antigos, importar, abrir, converter -->

**RTF** (Rich Text Format) é um tipo de arquivo de documento que quase todos os editores de texto conseguem ler e gravar. O JeffWrite abre arquivos **.rtf** e mantém o texto e a aparência deles.

## Abrir um arquivo RTF

Pressione **Ctrl+O** ou clique em **Arquivo > Abrir** e em **Procurar**. Escolha **Rich Text Format (*.rtf)** na lista de tipos de arquivo para ver somente esses arquivos, ou escolha o arquivo na lista de todos os documentos. Você também pode arrastar um arquivo .rtf para uma janela do JeffWrite ou informar o nome dele ao iniciar o JeffWrite pela linha de comando.

O JeffWrite identifica um arquivo RTF pelo que há dentro dele, e não pelo nome. Por isso, um arquivo com outra extensão também é aberto, desde que seja realmente um arquivo RTF.

## O que é importado

O JeffWrite lê estas partes de um arquivo RTF:

- Texto em qualquer idioma, com suas fontes, tamanhos, cores, negrito, itálico, sublinhado, tachado, sobrescrito, subscrito e realce.
- Configurações de parágrafo: alinhamento, recuos, espaçamento, paradas de tabulação, bordas, sombreamento e as configurações de manter com o próximo e manter linhas juntas.
- Estilos, listas numeradas e listas com marcadores.
- Tabelas, incluindo tabelas dentro de tabelas, bordas, sombreamento, larguras de coluna e células mescladas.
- Tamanho da página, margens, colunas, orientação da página, seções, cabeçalhos e rodapés.
- Notas de rodapé, notas de fim, campos de número de página, outros campos e indicadores.
- Alterações controladas: texto inserido, texto excluído, quebras de parágrafo inseridas ou excluídas e alterações de aparência. Uma **alteração controlada** é uma edição marcada por um revisor. Consulte [Controlar alterações e aceitá-las ou rejeitá-las](track-changes).
- Imagens nos formatos PNG, JPEG, EMF e WMF, além de quebras de página e de coluna.

Se parte do arquivo não puder ser lida, o JeffWrite informa o que deixou de fora. Alguns arquivos RTF contêm elementos que um documento do JeffWrite ainda não comporta, como desenhos ou equações. Esses elementos são deixados de fora, e o texto ao redor deles é mantido.

## Salvar as alterações

Você pode salvar um documento como arquivo .rtf: escolha **Rich Text Format (*.rtf)** em **Salvar Como** ou pressione **Ctrl+S** em um documento que veio de um arquivo .rtf. O arquivo mantém o texto e a aparência, os estilos, as listas, as tabelas, a configuração da página, os cabeçalhos e rodapés, os links e as imagens (PNG, JPEG e EMF). Ele não mantém notas de rodapé, notas de fim, comentários, alterações controladas, equações, formas nem caixas de texto. O JeffWrite informa o que deixou de fora ao salvar.

## Relacionados

- [Abrir, salvar, imprimir e exportar](open-save)
- [Bem-vindo ao JeffWrite](start)
