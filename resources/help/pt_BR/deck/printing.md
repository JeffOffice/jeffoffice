# Imprimir, PDF e imagens dos slides
<!-- keywords: imprimir, impressora, impressão, cópias, slides, páginas de anotações, folhetos, estrutura de tópicos, pdf, exportar, imagem, png, jpeg, jpg, figura, intervalo, slides ocultos, moldura, escala de cinza, preto e branco, visualização de impressão, papel, carta, a4, senha, criptografar, proteger com senha -->

Você pode imprimir sua apresentação, salvá-la como PDF ou salvar um slide como imagem. As três opções estão na tela Arquivo.

## Imprimir

1. Pressione Ctrl+P ou clique em **Arquivo > Imprimir**.
2. Em **Imprimir**, escolha o que será impresso: **Slides**, **Páginas de Anotações**, **Estrutura de Tópicos** ou **Folhetos** com 1, 2, 3, 4, 6 ou 9 slides por página.
3. Para folhetos com 4 ou mais slides por página, **Ordem** indica se os slides seguem da esquerda para a direita e depois de cima para baixo (**Horizontal**) ou de cima para baixo e depois da esquerda para a direita (**Vertical**).
4. Em **Intervalo**, escolha **Todos os slides** ou **Slide atual**, ou escolha **Slides** e digite os números na caixa: números de slides e intervalos separados por vírgulas, como 1-3, 5.
5. Marque o que for necessário: **Imprimir slides ocultos**, **Emoldurar slides** (uma linha fina ao redor de cada slide) e **Escala de cinza**.
6. Em **Lados**, escolha **Imprimir em um lado**, **Imprimir frente e verso, virar pela borda longa** (páginas que viram como um livro), **Imprimir frente e verso, virar pela borda curta** (páginas que viram como um bloco de notas) ou **Imprimir frente e verso manualmente**. As duas primeiras opções ficam esmaecidas quando sua impressora não consegue imprimir frente e verso. Aponte para uma delas para ver o motivo. **Manualmente** serve para essa impressora: o JeffDeck imprime a primeira, a terceira, a quinta e assim por diante, depois uma mensagem pede para você virar a pilha e colocá-la de volta na impressora. Ao clicar em **OK**, ele imprime as outras, começando pela última página. Com uma única página, não há o que virar. O JeffDeck lembra sua escolha.
7. Clique em **Imprimir**. O JeffDeck abre a janela de impressão do seu sistema, onde você escolhe a impressora e o número de cópias. A janela começa com a opção de lados que você escolheu. Um arquivo PDF criado por essa janela não tem lados.

A visualização à direita mostra cada página que será impressa, com setas para passar de uma página à outra. Ela é desenhada da mesma forma como a página será impressa. O JeffDeck lembra suas escolhas para a próxima vez.

## O que cada opção imprime

- **Slides** imprime um slide em cada página, no formato próprio do slide. O papel fica deitado quando o slide é largo.
- **Páginas de Anotações** coloca um slide no alto da página, com as anotações do orador abaixo dele.
- **Folhetos** colocam uma grade de slides em cada página, com cabeçalho e rodapé que trazem o título, a data e o número da página. Com 3 por página, há linhas ao lado de cada slide para anotações.
- **Estrutura de Tópicos** imprime o número e o título de cada slide, seguidos do texto do corpo, recuado conforme o nível.

Páginas de anotações, folhetos e a estrutura de tópicos são impressos em papel carta ou A4 em pé, conforme o padrão da sua região. Slides ocultos ficam de fora, a menos que você marque **Imprimir slides ocultos**.

## Salvar como PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. Uma janela pergunta o que incluir: as mesmas opções **Imprimir**, **Ordem** e **Intervalo** da impressão, e as mesmas três caixas de seleção. Ela mostra quantas páginas o arquivo terá. Clique em **Exportar…** e escolha um nome e um local.

O PDF tem texto real, com as fontes incorporadas, por isso fica igual em qualquer computador. O título dele é o título da apresentação ou o nome do arquivo.

Marque **PDF/A, para guardar por muito tempo** para criar um arquivo PDF/A-1b. Todas as fontes ficam dentro do arquivo, e os efeitos com transparência são nivelados, para que os slides continuem iguais por anos. O arquivo não tem marcações para leitores de tela. Um PDF com senha não pode ser PDF/A, por isso escolher uma senha desativa o PDF/A.

## Enviar uma apresentação por email

Clique em **Arquivo > Exportar > Criar um Email**. Escolha a apresentação como arquivo ou como PDF. O JeffDeck cria um arquivo de mensagem (.eml) com ela anexada e pergunta onde salvá-lo. Nada é enviado. Abra o arquivo no seu programa de email, informe os destinatários e envie. Uma apresentação com senha é anexada com a senha ainda aplicada. Um PDF pode ter uma senha própria.

## Proteger um PDF com senha

Na janela **Exportar para PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF terá que digitá-la. A senha realmente protege o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem queira contorná-las, por isso não conte com elas para manter um arquivo em sigilo.

Uma senha esquecida não pode ser recuperada, guarde-a em local seguro. A senha vale apenas para esse PDF e nunca é salva.

Ao inserir como imagem um PDF com senha, o JeffDeck pede a senha. Se ela estiver errada, ele pede de novo; clique em **Cancelar** para parar. O slide recebe uma cópia da página que não precisa de senha. Se o proprietário do PDF pedir que ele não seja copiado, o JeffDeck insere o arquivo mesmo assim e mostra um aviso sobre isso; esse pedido não é um bloqueio.

## Salvar um slide como imagem

Clique em **Arquivo > Exportar > Exportar Slide como Imagem**. O JeffDeck salva o slide atual como uma imagem PNG ou JPEG, com 1920 pixels de largura, sobre fundo branco. Ele sugere o nome da apresentação e o número do slide como nome do arquivo.

## Veja também

- [A tela Arquivo](file-screen)
- [Anotações do orador](notes)
- [Adicionar, alterar e excluir slides](slides)
