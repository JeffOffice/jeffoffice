# Imprimir e criar um PDF
<!-- keywords: páginas por folha, várias páginas por folha, n-up, páginas ímpares, páginas pares, frente e verso, dos dois lados, duplex, imprimir, impressora, visualizar, pdf, exportar, cópias, agrupar, páginas, intervalo de páginas, seleção, pasta de trabalho inteira, dimensionamento, ajustar, email, quebras de página, linhas tracejadas, salvar como pdf, dimensionar para o papel, tamanho do papel, visualização da planilha, senha, criptografar, proteger com senha -->

Escolha **Arquivo > Imprimir** (ou pressione **Ctrl+P**). A página abre com uma visualização das páginas que serão impressas, uma de cada vez. Use as setas abaixo dela para passar de uma página para outra, e **Ajustar página** ou os botões de zoom para ver mais ou menos de uma página. A visualização é desenhada da mesma forma que a impressora e o PDF, então o que você vê é o que você obtém.

## O que imprimir

- **O quê**: a planilha em que você está (ou todas as planilhas cujas guias você escolheu com Ctrl ou Shift, uma após a outra), a pasta de trabalho inteira (todas as planilhas que estão visíveis) ou a seleção. Planilhas ocultas são deixadas de fora. Uma planilha de gráfico ocupa uma página, com o gráfico tão grande quanto o espaço da página permitir dentro das margens.
- **Páginas**: todas, ou algumas, como **1-3, 5**.
- **Quais páginas**: **Todas as páginas**, **Somente páginas ímpares** ou **Somente páginas pares**. Use isso para imprimir frente e verso manualmente quando sua impressora não fizer isso sozinha. Primeiro imprima as páginas ímpares. Quando terminar, vire a pilha, coloque-a de volta na bandeja de papel e imprima novamente com **Somente páginas pares**.
- **Lados**: **Imprimir em um lado**, **Imprimir frente e verso, virar pela borda longa**, **Imprimir frente e verso, virar pela borda curta** ou **Imprimir frente e verso manualmente**. A borda longa serve para páginas que viram como um livro. A borda curta serve para páginas que viram como um bloco de notas. Uma impressora que não imprime frente e verso deixa essas duas opções esmaecidas. Aponte para uma delas para ver o motivo. **Manualmente** é para essa impressora: o JeffCrunch imprime a primeira, a terceira, a quinta e assim por diante, das páginas ou das folhas, se você imprimir várias páginas por folha. Depois, uma mensagem pede que você vire a pilha e a coloque de volta na bandeja de papel. Ao clicar em **OK**, ele imprime as outras, da última página para a primeira, para que caiam no verso. Se você imprimir apenas uma página, não há o que virar. Todas essas opções ficam esmaecidas ao salvar um PDF, porque um arquivo não tem lados. O JeffCrunch guarda sua escolha para a próxima vez.
- **Páginas por folha**: imprima 1, 2, 4, 6, 9 ou 16 páginas, reduzidas, em cada folha de papel. Cada página recebe uma moldura fina. A visualização ainda mostra uma página em tamanho real.
- **Cópias** e **Agrupadas** (um conjunto completo de cada vez, 1 2 3 1 2 3, em vez de as cópias de cada página juntas).
- **Orientação**, **Papel** e **Margens**, para usar nesta impressão em vez do que cada planilha indica.
- **Dimensionamento**: conforme cada planilha, tamanho real, a planilha inteira em uma página, todas as colunas em uma página ou todas as linhas em uma página.
- **Ignorar área de impressão**: imprime tudo o que tem algum conteúdo.
- **Configurar Página** abre a caixa de diálogo da planilha em que você está.

Abaixo da visualização, o JeffCrunch avisa quando algo precisa de sua atenção: uma planilha que precisa de mais páginas do que consegue gerar, uma planilha reduzida a ponto de as letras ficarem muito pequenas, ou uma planilha sem nada nela.

## Dimensionar para um tamanho de papel

**Dimensionar para o papel** torna cada página menor ou maior para caber no papel escolhido, mantendo sua forma e centralizando-a. Use para imprimir em um papel que sua impressora não tem, como uma página configurada para A4 em papel carta. **Sem dimensionamento** imprime cada página em seu próprio papel. A visualização mostra a folha de papel que será impressa.

## Várias páginas por folha

**Páginas por folha** coloca 2, 4, 6, 9 ou 16 páginas em cada folha de papel, cada uma menor e com moldura. A visualização mostra a folha inteira, e o rótulo abaixo dela indica qual folha você está vendo, como **Folha 1 de 2**. Use **Próxima** e **Anterior** para ver as outras folhas.

## A impressora

Escolha uma impressora na lista, ou **Salvar como PDF**. **Configurações da impressora** abre a caixa de diálogo do próprio sistema para as outras opções da impressora, como bandejas de papel. **Imprimir** envia as páginas.

## Exportar para PDF

**Arquivo > Exportar** cria um PDF da planilha em que você está, ou da pasta de trabalho inteira, sem escolher uma impressora. As letras continuam sendo letras (não imagens), então podem ser pesquisadas e permanecem nítidas ao aumentar o zoom.

**Exportar a pasta de trabalho inteira como PDF/A** cria um PDF no formato feito para guardar por anos. Todas as fontes são incorporadas ao arquivo, então ele será exibido da mesma forma por muito tempo. Programas que arquivam documentos pedem esse formato.

**Criar um Email** cria um arquivo de mensagem com sua pasta de trabalho dentro, ou um PDF dela. Nada é enviado. Abra o arquivo no seu programa de email, informe para quem ele é destinado e envie. Uma pasta de trabalho com senha permanece bloqueada na mensagem: quem a receber precisa da senha para abri-la. A senha em si nunca é colocada na mensagem. Um PDF da pasta de trabalho tem senha somente se você clicar em **Criptografar com senha…** na caixa, o que vale também para um PDF exportado. Informe a senha à pessoa por outro meio que não a mensagem.

## Proteger um PDF com senha

Em **Arquivo > Exportar**, clique em **Criptografar com senha…** antes de escolher um comando de PDF. Ela vale somente para o próximo PDF. Digite a senha duas vezes. Quem abrir o PDF terá de digitá-la. A senha realmente protege o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem quiser contorná-las, então não conte com elas para manter um arquivo em sigilo.

Uma senha esquecida não pode ser recuperada, então guarde-a em um lugar seguro. A senha é usada somente para esse PDF e nunca é salva. O comando **Exportar a pasta de trabalho inteira como PDF/A** fica esmaecido enquanto houver uma senha definida, porque um arquivo PDF/A não pode tê-la.

## Quebras de página na tela

Depois de abrir a tela de Impressão, a planilha na tela mostra onde as páginas começam: uma linha tracejada para uma quebra que o JeffCrunch calculou, e uma linha contínua para uma quebra que você definiu. Elas são atualizadas à medida que você altera a planilha. Desative-as em **Exibir > Linhas de Página**. (O modo de exibição **Visualização de Quebra de Página**, descrito em [Definir as páginas](page-layout), sempre as mostra.)

## O que ainda não é impresso

Anotações (comentários) são impressas após a última página da planilha quando a Configuração de Página da planilha indica **Listá-las após a última página**; elas ainda não são impressas onde estão nas células. Formatações condicionais ainda não são mostradas na tela, então também não são impressas.
