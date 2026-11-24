# Criar diapositivos a partir de uma estrutura de tópicos

<!-- keywords: diapositivos a partir de estrutura de tópicos, importar estrutura de tópicos, markdown, estrutura de tópicos em texto, rtf, docx, títulos para diapositivos, lista com marcas para diapositivos, iniciar apresentação a partir de texto, guardar estrutura de tópicos, exportar estrutura de tópicos, importar texto, criar diapositivos a partir de texto -->

Pode escrever uma apresentação como texto simples e deixar o JeffDeck transformá-lo em diapositivos. Cada diapositivo tem um título e um corpo, no esquema Título e Conteúdo, por isso a vista Estrutura de Tópicos, Repor Diapositivo e os temas funcionam nele como habitualmente.

## Adicionar os diapositivos à apresentação aberta

1. Clique em **Base > Diapositivos**, depois na pequena seta por baixo de **Novo Diapositivo**, e escolha **Diapositivos a Partir de Estrutura de Tópicos…**.
2. Escolha um ficheiro. Pode ser um ficheiro de texto (.txt), um ficheiro Markdown (.md), um ficheiro de texto formatado (.rtf) ou um ficheiro de documento (.docx).
3. Os diapositivos são adicionados após o diapositivo em que se encontra. Constituem um único passo, por isso um Ctrl+Z desfaz todos de uma vez.

## Ou iniciar uma nova apresentação a partir de um ficheiro

Escolha **Ficheiro > Abrir** e selecione o ficheiro de estrutura de tópicos. O JeffDeck cria uma nova apresentação a partir dele. O ficheiro nunca é alterado, e **Guardar** pede um nome.

## Como o ficheiro é lido

**Ficheiros de texto.** Uma linha que começa na margem esquerda é o título de um diapositivo. As linhas por baixo dele são o texto do diapositivo. Uma linha com mais avanço do que a anterior passa a um nível abaixo (use tabulações ou espaços). Uma marca (-, * ou +) ou um número no início de uma linha é omitido, porque o diapositivo tem as suas próprias marcas.

**Ficheiros Markdown.** O nível de título mais alto usado no ficheiro cria os diapositivos: # se o ficheiro tiver, caso contrário ##. Os títulos de nível inferior tornam-se linhas do texto do diapositivo, com um nível abaixo por cada passo. Uma linha com três traços (---) inicia um novo diapositivo. As listas tornam-se níveis. **Negrito**, *itálico*, `código` e ligações mantêm o seu aspeto. Um bloco de código entre linhas de três acentos graves torna-se uma caixa de texto com um tipo de letra de máquina de escrever. Uma imagem numa linha própria, escrita na forma de imagem do Markdown (um ponto de exclamação, a descrição entre parênteses retos e o nome do ficheiro entre parênteses), é colocada no diapositivo, mas apenas quando o ficheiro da imagem está na mesma pasta da estrutura de tópicos, ou numa pasta dentro dela. As imagens da Internet ou de outras pastas nunca são carregadas.

**Ficheiros de texto formatado e .docx.** Os parágrafos com um estilo de título criam os diapositivos: o estilo de título mais alto usado no ficheiro é o título, os títulos de nível inferior são linhas do texto, e cada outro parágrafo é uma linha por baixo do título anterior.

## Limites

Estes limites aplicam-se a todos os tipos de ficheiros de estrutura de tópicos, de texto, Markdown, texto formatado e .docx:

- Até 5 000 diapositivos. O que exceder esse número é omitido.
- Até 4 000 carateres numa linha. Uma linha mais longa é cortada, e o mesmo acontece a um parágrafo Markdown cujas linhas juntas excedam esse limite.
- Até 500 linhas de texto num diapositivo e 100 000 linhas no total.
- O texto de um diapositivo tem até 9 níveis, e uma linha com mais avanço do que isso é apresentada no nível mais profundo.
- Até 200 linhas de código num diapositivo.
- Até 100 imagens no total e 4 num diapositivo. Um ficheiro de imagem pode ter até 32 MB, e todos os ficheiros de imagem juntos até 64 MB. Uma imagem tem de ser um ficheiro PNG, JPEG, GIF, BMP, WebP ou TIFF, independentemente do nome que tenha, com até 16 milhões de píxeis (4 000 por 4 000), e todas as imagens juntas até 64 milhões.

O tamanho do próprio ficheiro também está limitado. Um ficheiro de texto ou Markdown pode ter até 10 MB, em UTF-8 ou UTF-16. Um ficheiro de texto formatado ou .docx pode ter até 96 MB. Um ficheiro que não seja de texto é recusado. As imagens de um ficheiro de texto formatado ou .docx não são colocadas nos diapositivos, e o JeffDeck indica quantas ficaram de fora.

Quando algo fica de fora, como uma imagem que não pôde ser usada ou texto além de um limite, o JeffDeck indica o que foi. Enquanto um ficheiro grande é lido, o JeffDeck continua a funcionar, e fechar a janela interrompe a leitura.

## Guardar uma cópia como estrutura de tópicos

Escolha **Ficheiro > Guardar uma Cópia** e selecione **Estrutura de Tópicos Markdown (*.md)**. O ficheiro contém o título e o texto de cada diapositivo, com negrito, itálico e ligações. Não inclui imagens, formas, tabelas, gráficos, outras caixas de texto, notas do orador nem o aspeto dos diapositivos. O JeffDeck indica isto ao guardar, e **Ficheiro > Informações > Verificar Compatibilidade** também o lista.

## Ver também

- [Adicionar, alterar e eliminar diapositivos](slides)
- [Reutilizar diapositivos de outra apresentação](reuse)
- [Vistas](views)
