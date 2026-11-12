# Leitores de tela, alto contraste e texto alternativo
<!-- keywords: acessibilidade, leitor de tela, narrador, cego, baixa visão, alto contraste, modo escuro, tema escuro, contorno de foco, texto alternativo, descrição de imagem, zoom, ampliar, somente teclado -->

O JeffPub foi criado para que pessoas que usam leitores de tela, o teclado ou cores de alto contraste também possam usá-lo.

## Leitores de tela

Um **leitor de tela** é um programa que lê a tela em voz alta. O JeffPub funciona com leitores de tela como o Narrador no Windows. Cada controle tem um nome e uma descrição. Isso inclui as guias, botões, galerias e botões de cor da faixa de opções, a lista de Páginas, a página, os painéis de tarefas e a barra de status.

Os objetos da página também são lidos. Na página, pressione **Tab** para selecionar o próximo objeto: o leitor de tela diz que tipo de objeto é e o que contém, como "Caixa de texto: Promoção de primavera" ou "Imagem: um celeiro vermelho", depois onde ele está e qual é o seu tamanho. Uma imagem sem texto alternativo é lida como "Imagem, sem texto alternativo", para que você saiba quais ainda precisam dele. Pressione **Enter** para digitar em uma caixa de texto; o leitor de tela pode então ler o texto, a linha ou a palavra em que está o cursor e o que você selecionar.

- A lista de Páginas lê cada página como "Página 1", "Página 2" e assim por diante. Se você deu um nome a uma página, o nome vem em seguida, como em "Página 3, Capa".
- A área da página se chama "Página". A descrição dela informa as teclas a usar: Tab seleciona o próximo objeto, as setas o movem, Enter edita o texto dele e Delete o remove.
- As guias são lidas como guias da faixa de opções. As guias que aparecem ao selecionar algo também informam o grupo a que pertencem, como Ferramentas de Imagem.

## Usar o teclado

Você pode acessar a faixa de opções, a lista de Páginas e os objetos da página somente com o teclado. Pressione F6 para alternar entre a faixa de opções, a lista de Páginas e a página. Pressione Alt para ver as Dicas de Tecla. Um contorno colorido mostra qual controle tem o foco. Veja [Usar o JeffPub com o teclado](keyboard).

## Alterar as cores

- **Escuro ou claro**: clique em **Arquivo > Opções** e, na guia **Geral**, defina **Tema** como **Claro**, **Escuro** ou **Usar configuração do sistema**. A alteração acontece imediatamente.
- **Alto contraste**: no Windows, quando você ativa um tema de alto contraste, o JeffPub o acompanha. Ele usa as cores escolhidas pelo Windows e muda conforme você ativa ou desativa o tema.

## Tornar as imagens mais fáceis de entender

O **texto alternativo** é uma breve descrição de uma imagem ou de outro objeto. Os leitores de tela o leem em voz alta. O JeffPub também o inclui nas páginas da Web e nos e-books que você salvar.

1. Clique com o botão direito na imagem ou no objeto.
2. Clique em **Formatar Objeto…**.
3. Clique na guia **Texto Alt**.
4. Digite uma descrição na caixa **Texto alternativo**.
5. Clique em **OK**.

Diga, em poucas palavras, o que a imagem mostra, como "Duas crianças plantando uma árvore". Ícones e códigos de barras que você inserir recebem texto alternativo automaticamente.

## Verificar a página

Clique em **Revisão > Verificar Acessibilidade** para listar o que pode ser difícil de usar para algumas pessoas. Além de imagens e formas sem texto alternativo, a verificação encontra mais três situações.

- **Texto difícil de ler.** O texto precisa de *contraste* suficiente com o que está atrás dele: o preenchimento da caixa, ou a cor da página quando a caixa não tem preenchimento. A verificação calcula o contraste como uma proporção. Uma proporção de 4,5 para 1 basta para texto comum, e 3 para 1 basta para texto grande (18 pontos ou mais, ou 14 pontos ou mais em negrito). Por exemplo, o cinza #777777 sobre branco tem proporção de 4,48 para 1, pouco abaixo do necessário, e #767676 tem 4,54 para 1. Escolha uma cor de texto mais escura ou um fundo mais claro. Quando o texto está sobre uma imagem, a verificação não consegue determinar o contraste, então exibe uma dica para verificar o contraste a olho.
- **Objetos lidos em uma ordem diferente.** Um leitor de tela e um PDF com marcações leem os objetos de uma página de trás para frente na pilha. Se essa ordem não for de cima para baixo e, depois, da esquerda para a direita, a verificação lista como dicas os objetos fora de lugar. Clique em **Ordem de Leitura** para ver os objetos na ordem em que são lidos. Selecione um deles e clique em **Ler Antes** ou **Ler Depois**, ou arraste-o. Cada movimento é uma etapa de **Desfazer**, e ele também altera qual objeto fica na frente.
- **Tabela sem linha de cabeçalho.** Um leitor de tela usa a primeira linha de uma tabela para nomear as colunas. Uma tabela cuja primeira linha não está marcada como linha de cabeçalho aparece na lista como aviso. Clique no problema e, em seguida, em **Usar a primeira linha como cabeçalho** para ativar a linha de cabeçalho. Essa ação também é uma etapa de **Desfazer**.

## Aumentar a página

Amplie para ler textos pequenos. Use os controles de **Zoom** na barra de status, mantenha Ctrl pressionado e gire a roda do mouse, ou escolha um tamanho na caixa **Zoom** da guia **Exibir**. O controle deslizante de zoom vai de cerca de 10% a 1600%.

## Relacionados

- [Usar o JeffPub com o teclado](keyboard)
- [Adicionar e ajustar imagens](pictures)
- [Alterar as configurações do JeffPub](options)
- [Salvar como imagem, página da Web ou e-book](export)
