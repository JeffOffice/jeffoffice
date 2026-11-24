# Criar slides a partir de um contorno

<!-- keywords: slides a partir de contorno, importar contorno, markdown, contorno de texto, rtf, docx, títulos para slides, lista de marcadores para slides, iniciar apresentação a partir de texto, salvar contorno, exportar contorno, criar slides a partir de texto, importar texto, esboço, estrutura de tópicos -->

Você pode escrever uma apresentação como texto simples e deixar que o JeffDeck a transforme em slides. Cada slide tem um título e um corpo, no layout Título e Conteúdo, para que o Modo de Exibição de Estrutura de Tópicos, Redefinir Slide e os temas funcionem nele normalmente.

## Adicionar os slides à apresentação aberta

1. Clique em **Página Inicial > Slides**, depois na pequena seta abaixo de **Novo Slide** e escolha **Slides a Partir do Contorno…**.
2. Escolha um arquivo. Pode ser um arquivo de texto (.txt), um arquivo Markdown (.md), um arquivo de texto formatado (.rtf) ou um arquivo de documento (.docx).
3. Os slides são adicionados depois do slide atual. A operação é uma única etapa, então um Ctrl+Z desfaz todos eles de uma vez.

## Ou iniciar uma nova apresentação a partir dele

Escolha **Arquivo > Abrir** e selecione o arquivo de contorno. O JeffDeck cria uma nova apresentação a partir dele. O arquivo nunca é alterado, e **Salvar** pede um nome.

## Como o arquivo é lido

**Arquivos de texto.** Uma linha que começa na margem esquerda é o título de um slide. As linhas abaixo dele são o texto do slide. Uma linha recuada além da anterior passa a um nível mais abaixo (use tabulações ou espaços). Um marcador (-, * ou +) ou um número no início da linha é descartado, pois o slide tem marcadores próprios.

**Arquivos Markdown.** O nível de título mais alto usado no arquivo cria os slides: # se o arquivo tiver esse nível, caso contrário, ##. Títulos menores tornam-se linhas do texto do slide, um nível mais abaixo para cada passo. Uma linha com três traços (---) inicia um novo slide. Listas viram níveis. **Negrito**, *itálico*, `código` e links mantêm a aparência. Um bloco de código entre linhas de três crases vira uma caixa de texto com fonte de máquina de escrever. Uma imagem em uma linha própria, escrita na forma de imagem do Markdown (um ponto de exclamação, a descrição entre colchetes e o nome do arquivo entre parênteses), é colocada no slide, mas somente quando o arquivo da imagem está na mesma pasta do contorno, ou em uma pasta dentro dela. Imagens da internet ou de outras pastas nunca são carregadas.

**Arquivos de texto formatado e .docx.** Parágrafos em um estilo de Título criam os slides: o estilo de título mais alto usado no arquivo é o título, os títulos menores são linhas do texto, e cada outro parágrafo é uma linha abaixo do título anterior.

## Limites

Estes limites valem para todos os tipos de arquivo de contorno, texto, Markdown, texto formatado e .docx:

- Até 5.000 slides. O que vier depois disso é deixado de fora.
- Até 4.000 caracteres em uma linha. Uma linha mais longa é cortada, o mesmo acontecendo com um parágrafo Markdown cujas linhas se juntem em um texto maior que esse.
- Até 500 linhas de texto em um slide e 100.000 linhas no total.
- O texto de um slide tem até 9 níveis, e uma linha recuada além disso é mostrada no nível mais profundo.
- Até 200 linhas de código em um slide.
- Até 100 imagens no total e 4 em um slide. Um arquivo de imagem pode ter até 32 MB, e todos os arquivos de imagem juntos até 64 MB. Uma imagem precisa ser um arquivo PNG, JPEG, GIF, BMP, WebP ou TIFF, independentemente do nome, com até 16 milhões de pixels (4.000 por 4.000), e todas as imagens juntas até 64 milhões de pixels.

O tamanho do próprio arquivo também tem limite. Um arquivo de texto ou Markdown pode ter até 10 MB, em UTF-8 ou UTF-16. Um arquivo de texto formatado ou .docx pode ter até 96 MB. Um arquivo que não seja de texto é recusado. As imagens de um arquivo de texto formatado ou .docx não são colocadas nos slides, e o JeffDeck informa quantas foram deixadas de fora.

Quando algo é deixado de fora, como uma imagem que não pôde ser usada ou um texto além de um limite, o JeffDeck informa o que foi. Enquanto um arquivo grande é lido, o JeffDeck continua funcionando, e fechar a janela interrompe a leitura.

## Salvar uma cópia como contorno

Escolha **Arquivo > Salvar uma Cópia** e selecione **Contorno Markdown (*.md)**. O arquivo guarda o título e o texto de cada slide, com negrito, itálico e links. Ele deixa de fora imagens, formas, tabelas, gráficos, outras caixas de texto, anotações do orador e a aparência dos slides. O JeffDeck informa isso ao salvar, e **Arquivo > Informações > Verificar Compatibilidade** também lista essas perdas.

## Relacionados

- [Adicionar, alterar e excluir slides](slides)
- [Reutilizar slides de outra apresentação](reuse)
- [Modos de exibição](views)
