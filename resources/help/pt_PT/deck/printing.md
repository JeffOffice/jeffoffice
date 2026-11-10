# Imprimir, PDF e imagens dos diapositivos
<!-- keywords: imprimir, impressora, impressão, cópias, diapositivos, páginas de notas, folhetos, estrutura de tópicos, pdf, exportar, imagem, png, jpeg, jpg, imagem, intervalo, diapositivos ocultos, moldura, escala de cinzentos, preto e branco, pré-visualizar impressão, papel, a4, palavra-passe, encriptar, proteger com palavra-passe -->

Pode imprimir a sua apresentação, guardá-la como PDF ou guardar um diapositivo como imagem. As três opções estão no ecrã Ficheiro.

## Imprimir

1. Prima Ctrl+P, ou clique em **Ficheiro > Imprimir**.
2. Em **Imprimir o quê**, escolha o que sai: **Diapositivos**, **Páginas de Notas**, **Estrutura de Tópicos** ou **Folhetos** com 1, 2, 3, 4, 6 ou 9 diapositivos por página.
3. Nos folhetos com 4 ou mais diapositivos por página, **Ordem** indica se os diapositivos seguem ao longo da página e depois para baixo (**Horizontal**) ou para baixo e depois ao longo da página (**Vertical**).
4. Em **Intervalo**, escolha **Todos os diapositivos** ou **Diapositivo atual**, ou escolha **Diapositivos** e escreva os números na caixa: números e intervalos de diapositivos separados por vírgulas, como 1-3, 5.
5. Confirme o que precisa: **Imprimir diapositivos ocultos**, **Delimitar diapositivos** (uma linha fina à volta de cada diapositivo) e **Escala de cinzentos**.
6. Em **Lados**, escolha **Imprimir num lado**, **Imprimir nos dois lados, virar pelo lado longo** (páginas que se viram como um livro), **Imprimir nos dois lados, virar pelo lado curto** (páginas que se viram como um bloco de notas) ou **Imprimir nos dois lados manualmente**. As duas primeiras opções ficam a cinzento quando a impressora não consegue imprimir nos dois lados. Aponte para uma delas para ver o motivo. **Manualmente** destina-se a esse tipo de impressora: o JeffDeck imprime a primeira, a terceira, a quinta e assim sucessivamente, depois uma mensagem pede que volte a pilha ao contrário e a coloque de novo na impressora. Quando clicar em **OK**, imprime as restantes, começando pela última página. Com uma única página, não há nada para virar. O JeffDeck memoriza a sua escolha.
7. Clique em **Imprimir**. O JeffDeck abre a janela de impressão do sistema, onde escolhe a impressora e o número de cópias. A janela começa com a opção de lados que escolheu. Um ficheiro PDF criado a partir dessa janela não tem lados.

A pré-visualização à direita mostra cada página que vai sair, com setas para passar de página em página. É desenhada da mesma forma como a página será impressa. O JeffDeck memoriza as suas escolhas para a próxima vez.

## O que cada opção imprime

- **Diapositivos** imprime um diapositivo em cada página, com a forma própria do diapositivo. O papel fica na horizontal quando o diapositivo é largo.
- **Páginas de Notas** coloca um diapositivo no topo de uma página, com as respetivas notas do orador por baixo.
- **Folhetos** colocam uma grelha de diapositivos em cada página, com um cabeçalho e um rodapé que incluem o título, a data e o número de página. Com 3 por página, há linhas ao lado de cada diapositivo para escrever.
- **Estrutura de Tópicos** imprime o número e o título de cada diapositivo, seguidos do respetivo texto, com avanços consoante o nível.

As páginas de notas, os folhetos e a estrutura de tópicos são impressos em papel Carta ou A4 na vertical, consoante o que a sua região utiliza. Os diapositivos ocultos não são impressos, a menos que marque **Imprimir diapositivos ocultos**.

## Guardar como PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Abre-se uma janela que pergunta o que incluir: as mesmas escolhas de **Imprimir o quê**, **Ordem** e **Intervalo** da impressão, e as mesmas três caixas de verificação. Mostra quantas páginas terá o ficheiro. Clique em **Exportar…** e escolha um nome e um local.

O PDF tem texto real, com os tipos de letra incorporados, pelo que tem o mesmo aspeto em qualquer computador. O título é o título da apresentação ou o nome do ficheiro.

Marque **PDF/A, para conservação a longo prazo** para criar um ficheiro PDF/A-1b. Todos os tipos de letra ficam incluídos no ficheiro e os efeitos de transparência são achatados, pelo que os diapositivos continuam com o mesmo aspeto ao longo dos anos. Este ficheiro não tem marcação para leitores de ecrã. Um PDF com palavra-passe não pode ser um ficheiro PDF/A, por isso escolher uma palavra-passe desativa o PDF/A.

## Enviar uma apresentação por correio eletrónico

Clique em **Ficheiro > Exportar > Criar um Correio Eletrónico**. Escolha a apresentação como ficheiro ou como PDF. O JeffDeck cria um ficheiro de mensagem (.eml) com a apresentação anexada e pede onde a guardar. Nada é enviado. Abra o ficheiro no seu programa de correio eletrónico, indique os destinatários e envie-o. Uma apresentação que tenha palavra-passe é anexada com a palavra-passe ainda ativa. Um PDF pode ter uma palavra-passe própria.

## Proteger um PDF com palavra-passe

Na janela **Exportar para PDF**, clique em **Encriptar com uma palavra-passe…**. Escreva a palavra-passe duas vezes. Quem abrir o PDF terá, depois, de a escrever. A palavra-passe protege realmente o ficheiro: as páginas são cifradas com encriptação forte (AES-256), e ninguém as consegue ler sem ela.

Também pode marcar **Restringir impressão, cópia e edição**, indicar uma segunda palavra-passe e escolher o que é permitido. Essas limitações são apenas um pedido. A maioria dos leitores de PDF respeita-as, mas não impedem quem queira contorná-las, por isso não confie nelas para manter um ficheiro privado.

Uma palavra-passe esquecida não pode ser recuperada, por isso guarde-a num local seguro. A palavra-passe é usada apenas para esse PDF e nunca é guardada.

Ao inserir como imagem um PDF com palavra-passe, o JeffDeck pede a palavra-passe. Se estiver errada, volta a pedi-la; clique em **Cancelar** para parar. O diapositivo recebe uma cópia da página que não necessita de palavra-passe. Se o proprietário do PDF tiver pedido que não seja copiado, o JeffDeck coloca-o na mesma e mostra uma nota a informar; esse pedido não é um bloqueio.

## Guardar um diapositivo como imagem

Clique em **Ficheiro > Exportar > Exportar Diapositivo como Imagem**. O JeffDeck guarda o diapositivo atual como imagem PNG ou JPEG, com 1920 pixels de largura, sobre um fundo branco. Sugere como nome do ficheiro o nome da apresentação e o número do diapositivo.

## Ver também

- [O ecrã Ficheiro](file-screen)
- [Notas do orador](notes)
- [Adicionar, alterar e eliminar diapositivos](slides)
