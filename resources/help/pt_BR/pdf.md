# Arquivos PDF e gráficas

<!-- keywords: pdf, criar pdf, salvar como pdf, exportar pdf, pdf/x, pdf/a, arquivamento, arquivar, gráfica, gráfica comercial, cmyk, sangria, marcas de corte, perfil de cor, cor especial, sobreimpressão, senha, criptografar, proteger com senha, pdf com senha, pdf/x, pdf/a -->

Um **PDF** é um arquivo que tem a mesma aparência em qualquer computador. Use-o para enviar uma publicação por email, publicá-la na internet ou enviá-la a uma gráfica.

## Criar um PDF

1. Clique em **Arquivo > Exportar** e depois em **Criar PDF**.
2. Em **Otimizar para**, escolha uma qualidade. **dpi** significa pontos por polegada. Mais pontos deixam as imagens mais nítidas e os arquivos maiores.
3. Em **Páginas**, escolha **Todas as páginas**, **Página atual** ou **Páginas de** uma página **até** outra.
4. Clique em **Salvar PDF…**, digite um nome e clique em **Salvar**.

| Escolha | Imagens | Ideal para |
|---|---|---|
| Tamanho mínimo | 96 dpi | Visualização na tela |
| Padrão | 150 dpi | Compartilhamento na internet |
| Impressão de alta qualidade | 300 dpi | Impressoras de mesa |
| Gráfica comercial | Resolução total | Uma gráfica. Acrescenta marcas de corte, marcas de sangria, marcas de registro, barras de cores e informações do trabalho. |

Outras opções:

- **Incluir propriedades do documento** coloca o título, o autor, o assunto e as palavras-chave no PDF.
- **Tags de estrutura do documento para acessibilidade** (ativada por padrão) cria um PDF *marcado*. As marcas informam a um leitor de tela o que é título, parágrafo, imagem ou tabela, e a ordem de leitura: a ordem dos objetos em cada página, que você pode ver e alterar na lista **Ordem de Leitura** do Verificador de Acessibilidade. Uma imagem é lida com o seu texto alternativo. Um arquivo PDF/X não tem marcas.
- **Criar indicadores** (ativada por padrão) coloca um indicador no PDF para cada página. Ele recebe o nome da página ou "Página 3", no caso de uma página sem nome. Um leitor de PDF lista os indicadores ao lado das páginas.
- **Abrir o PDF depois de salvá-lo** mostra o PDF no seu visualizador imediatamente.
- **Folhas do livreto** aparece para os livretos. Ela coloca duas páginas em cada lado, na ordem de dobra. Desmarque para ter uma página por página do PDF.

## PDF/A para guardar registros

**PDF/A** é um PDF feito para durar anos. Ele incorpora todas as fontes e usa cores padrão, sem efeitos de transparência. Marque **PDF/A para arquivamento de longo prazo** ou clique em **Arquivo > Exportar** e depois em **Criar PDF/A para Arquivamento**.

## PDF/X para uma gráfica

**PDF/X** é um PDF mais rigoroso, que as gráficas costumam pedir. As cores são dadas como quantidades de tinta, as fontes ficam incorporadas e o corte e a sangria são marcados. A **sangria** é a imagem ou cor extra que ultrapassa a borda da página, para que o corte não deixe uma borda branca.

1. Clique em **Arquivo > Exportar** e depois em **Criar PDF/X para uma Gráfica Comercial**. Ou marque **PDF/X para uma gráfica comercial** na janela **Criar PDF**. Não é possível usá-lo com PDF/A.
2. Em **Condição de impressão**, escolha a que a sua gráfica indicar.
3. Se a gráfica pedir marcas de corte e sangria, escolha também **Gráfica comercial** em **Otimizar para**.
4. Clique em **Salvar PDF…**.

As condições são:

- **PDF/X-1a** para offset rotativo dos EUA (SWOP) ou offset europeu (FOGRA39). Ele nivela os efeitos de transparência.
- **PDF/X-4** para papel revestido premium (FOGRA51, PSO Coated v3) ou com o perfil de cor da sua própria impressora (um arquivo .icc que você escolhe). Ele mantém os efeitos de transparência.

O perfil de cor PSO Coated v3 não acompanha o JeffPub. Na primeira vez que você o usar, o JeffPub pergunta antes de baixá-lo, uma única vez, da European Color Initiative.

Antes de criar o arquivo, o JeffPub verifica se há imagens abaixo de 150 ppi (pixels por polegada) no tamanho impresso e se há fontes ausentes neste computador. Clique em **Criar PDF Mesmo Assim**, **Abrir Verificador de Design** ou **Cancelar**.

## Proteger um PDF com senha

Na janela **Criar PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF terá de digitá-la. A senha realmente protege o arquivo: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem queira contorná-las; portanto, não conte com elas para manter um arquivo privado.

Uma senha esquecida não pode ser recuperada, por isso guarde-a em um lugar seguro. A senha é usada somente para esse PDF e nunca é salva. Um PDF com senha não pode ser PDF/A nem PDF/X, por isso essas duas opções ficam desativadas enquanto houver uma senha definida. Em **Arquivo > Compartilhar**, **Criptografar com senha…** acima de **Enviar como PDF** coloca uma senha no PDF de um email.

Ao inserir como imagem um PDF que tem senha, o JeffPub pede a senha. Se ela estiver errada, o JeffPub pergunta de novo; clique em **Cancelar** para parar. A página na sua publicação é uma cópia e não precisa de senha. Se o proprietário do PDF pedir que ele não seja copiado, o JeffPub insere o arquivo mesmo assim e mostra um aviso informando isso; esse pedido não é um bloqueio.

Uma imagem vinculada a um PDF com senha aparece como **Não atualizada** quando você abre a publicação, e o JeffPub nunca pede a senha por conta própria. Clique em **Atualizar Vínculo** para ser solicitado.

## Configurar as cores para uma gráfica

Clique em **Arquivo > Informações** e depois no cartão **Configurações de Impressão Comercial**:

- **Modelo de cores** é **RGB**, **Cor única (especial)**, **Cores especiais**, **Cores de processo (CMYK)** ou **Cores de processo e especiais**. As cores de processo são as tintas ciano, magenta, amarela e preta. Uma cor especial é uma tinta própria que a gráfica prepara. Adicione cada uma em **Cores especiais**.
- **Sobreimpressão de preto em arquivos para gráfica (PDF/X)** faz o preto imprimir sobre as cores que estão embaixo dele, de modo que uma chapa deslocada não deixe borda branca. Você escolhe o que ele abrange: textos abaixo de um tamanho, linhas ou preenchimentos.

## Veja também

- [Imprimir uma publicação](printing)
- [Salvar como imagem, página da Web ou e-book](export)
- [Verificar o design](design-checker)
