# Leitores de ecrã, alto contraste e texto alternativo
<!-- keywords: acessibilidade, leitor de ecrã, narrador, cego, baixa visão, alto contraste, modo escuro, tema escuro, contorno de foco, texto alternativo, descrição de imagem, zoom, ampliar, apenas teclado, invisual, deficiência visual -->

O JeffPub foi concebido para que as pessoas que utilizam leitores de ecrã, o teclado ou cores de alto contraste também o possam utilizar.

## Leitores de ecrã

Um **leitor de ecrã** é um programa que lê em voz alta o conteúdo do ecrã. O JeffPub funciona com leitores de ecrã como o Narrador no Windows. Cada controlo tem um nome e uma descrição. Isto inclui os separadores, botões, galerias e botões de cor do friso, a lista de Páginas, a página, os painéis de tarefas e a barra de estado.

Os objetos na página também são lidos. Na página, prima **Tab** para selecionar o objeto seguinte: o leitor de ecrã indica o tipo de objeto e o que contém, como "Caixa de texto: Saldos de primavera" ou "Imagem: um celeiro vermelho", e depois onde está e qual o seu tamanho. Uma imagem sem texto alternativo é lida como "Imagem, sem texto alternativo", para que saiba quais ainda precisam de um. Prima **Enter** para escrever numa caixa de texto; o leitor de ecrã pode então ler o respetivo texto, a linha ou a palavra onde está o cursor, e o que tiver selecionado.

- A lista de Páginas lê cada página como "Página 1", "Página 2", e assim sucessivamente. Se tiver dado um nome a uma página, o nome segue-se, como em "Página 3, Capa".
- A área da página chama-se "Página". A respetiva descrição indica as teclas a utilizar: Tab seleciona o objeto seguinte, as teclas de seta movem-no, Enter edita o respetivo texto e Delete elimina-o.
- Os separadores são lidos como separadores do friso. Os separadores que aparecem ao selecionar algo indicam também o grupo a que pertencem, como Ferramentas de Imagem.

## Utilizar o teclado

Pode chegar ao friso, à lista de Páginas e aos objetos na página apenas com o teclado. Prima F6 para mover o foco entre o friso, a lista de Páginas e a página. Prima Alt para ver as Sugestões de Teclas. Um contorno colorido mostra qual o controlo que tem o foco. Consulte [Utilizar o JeffPub com o teclado](keyboard).

## Alterar as cores

- **Escuro ou claro**: clique em **Ficheiro > Opções** e, no separador **Geral**, defina **Tema** como **Claro**, **Escuro** ou **Utilizar definição do sistema**. A alteração é feita de imediato.
- **Alto contraste**: no Windows, quando ativa um tema de alto contraste, o JeffPub acompanha-o. Utiliza as cores escolhidas pelo Windows e muda à medida que ativa ou desativa o tema.

## Tornar as imagens mais fáceis de compreender

O **texto alternativo** (texto alternativo) é uma breve descrição de uma imagem ou de outro objeto. Os leitores de ecrã leem-no em voz alta. O JeffPub também o inclui nas páginas Web e nos livros eletrónicos que guardar.

1. Clique com o botão direito na imagem ou no objeto.
2. Clique em **Formatar Objeto…**.
3. Clique no separador **Texto Alternativo**.
4. Escreva uma descrição na caixa **Texto alternativo**.
5. Clique em **OK**.

Descreva o que a imagem mostra, em poucas palavras, como "Duas crianças a plantar uma árvore". Os ícones e códigos de barras que inserir recebem texto alternativo automaticamente.

## Verificar a página

Clique em **Rever > Verificar Acessibilidade** para listar o que pode ser difícil de utilizar para algumas pessoas. Para além das imagens e formas sem texto alternativo, o verificador encontra mais quatro situações.

- **Texto difícil de ler.** O texto precisa de contraste suficiente com o que está por trás: o preenchimento da respetiva caixa ou a cor da página, quando a caixa não tem preenchimento. O verificador calcula o contraste como um rácio. Um rácio de 4,5 para 1 é suficiente para texto normal, e 3 para 1 para texto grande (18 pontos ou mais, ou 14 pontos ou mais a negrito). Por exemplo, o cinzento #777777 sobre branco tem 4,48 para 1, ficando ligeiramente abaixo, e #767676 tem 4,54 para 1. Escolha uma cor de texto mais escura ou um fundo mais claro. Quando o texto está sobre uma imagem, o verificador não consegue determiná-lo, pelo que acrescenta uma sugestão para verificar o contraste a olho.
- **Objetos lidos por uma ordem diferente.** Um leitor de ecrã e um PDF com etiquetas leem os objetos de uma página desde a parte de trás da pilha até à frente. Se essa ordem não for de cima para baixo e, depois, da esquerda para a direita, o verificador lista como sugestões os objetos que estão fora de lugar. Clique em **Ordem de Leitura** para ver os objetos pela ordem em que são lidos. Selecione um e clique em **Ler Antes** ou **Ler Depois**, ou arraste-o. Cada movimento corresponde a um passo de Anular, e também altera o objeto que está à frente.
- **Tabela sem linha de cabeçalho.** Um leitor de ecrã utiliza a primeira linha de uma tabela para dar nome às colunas. Uma tabela cuja primeira linha não está marcada como linha de cabeçalho é listada como aviso. Clique no problema e, depois, clique em **Utilizar a primeira linha como cabeçalho** para ativar a linha de cabeçalho. Trata-se de um passo de Anular.
- **Tabela com células unidas.** Um leitor de ecrã pode perder a sua posição numa tabela quando há células unidas, por isso uma tabela com uma célula unida é listada como aviso. Mantenha cada célula de uma tabela inteira: separe as células unidas ou utilize duas tabelas.
- **Ligação que não indica para onde leva.** Um leitor de ecrã pode ler as ligações de uma página como uma lista, por isso as palavras de cada ligação devem indicar a página que abre. Uma ligação cujas palavras são o próprio endereço, são apenas "aqui", "clique aqui", "saiba mais" ou uma expressão semelhante, ou estão vazias, é listada como aviso. Altere as palavras, por exemplo para "Relatório anual" em vez de "clique aqui". As primeiras 50 são listadas uma a uma, e as restantes são contadas numa única linha.

## Ampliar a página

Amplie para ler texto pequeno. Utilize os controlos de **Zoom** na barra de estado, mantenha Ctrl premido enquanto roda a roda do rato, ou escolha um tamanho na caixa **Zoom** do separador **Ver**. O cursor de zoom vai de cerca de 10% a 1600%.

## Consulte também

- [Utilizar o JeffPub com o teclado](keyboard)
- [Adicionar e ajustar imagens](pictures)
- [Alterar as definições do JeffPub](options)
- [Guardar como imagem, página Web ou livro eletrónico](export)
